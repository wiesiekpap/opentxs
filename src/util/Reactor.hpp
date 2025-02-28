// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"
#ifndef UTIL_REACTOR_HPP_
#define UTIL_REACTOR_HPP_

//
// Reactor encapsulates a processing loop for an active object.
// Tasks are submitted as messages (post, post_at and defer),
// or as arbitrary parameterless lambdas. The lambdas take precedence over
// messages.
//
// Task submission interface
// -------------------------
// - post() to queue a message for processing,
// - post_at() to queue a message for scheduled execution,
// - synchronize() to queue a parameterless lambda for priority execution, wait
// for the result or for the completion, if the submitted function returns void.
// - allow_command_processing() must be called to enable synchronized command
// processing. Actor calls it after handling the init message. Worker leaves
// calling it to the subclass.
//
// Task execution interface
// ------------------------
// - synchronized lambdas are executed directly,
// - handle() must be implemented by a subclass to process messages,
// NOTE: a message can be submitted with an integer index value. It will be
// passed to handle() with the original index value.
// - start() has to be called to activate the reactor thread,
// - stop() may be called to deactivate the reactor thread.
//
// Auxillary interface
// -------------------
// - name() to identify the instance,
// - in_reactor_thread() to check if the call has been made in the reactor
// thread.
//
// Protected subclass interface
// ----------------------------
// - handle() to pass message for processing,
// - constructor and destructor
// - defer() and flush_cache() are Actor/Worker specific.
//

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "util/threadutil.hpp"
#include "util/timed.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{

// Classes used internally for synchronization.
// TODO Hide as a private definition in Reactor once gcc 12 has been deployed
// for all platforms.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79501

class HiPCommand
{
public:
    HiPCommand() = default;
    virtual ~HiPCommand() {}
    virtual auto exec() -> void = 0;
    virtual auto dispose() -> void = 0;
};

template <typename F>
class SynchronizableCommand : public HiPCommand
{
public:
    explicit SynchronizableCommand(F f)
        : HiPCommand{}
        , disarmed_{false}
        , task_{[this, f] {
            bool disarmed = disarmed_.exchange(false);
            if (!disarmed) { throw std::runtime_error("uncompleted task"); }
            return f();
        }}
    {
    }
    auto exec() -> void final
    {
        disarmed_ = true;
        task_();
    }
    auto dispose() -> void final { task_(); }
    auto get_future() { return task_.get_future(); }

private:
    using R = typename std::invoke_result<F>::type;
    std::atomic<bool> disarmed_;
    std::packaged_task<R()> task_;
};
template <typename F>
SynchronizableCommand(F) -> SynchronizableCommand<F>;

class Reactor : public ThreadDisplay
{
public:
    // Run a functor in the reactor thread.
    // This is intended to be taking a lambda with an appropriate
    // capture.
    // NOTE: if the reactor has been stopped or it has not yet been started, the
    // lambda will be invoked directly.
    template <typename Functor>
    auto synchronize(Functor&& f)
    {
        try {
            if (in_reactor_thread() || !active_) { return f(); }
            tdiag(typeid(this), "Reactor::synchronize");
            std::unique_ptr<SynchronizableCommand<Functor>> fc(
                new SynchronizableCommand(std::move(f)));
            auto ft = fc->get_future();
            {
                std::unique_lock<std::mutex> lk(mtx_queue_state);
                if (active_) {
                    command_queue_.push(std::move(fc));
                    notify();
                } else {
                    fc->exec();
                    fc.reset();
                }
            }
            return ft.get();
        } catch (const std::exception& e) {
            std::cerr << ThreadMonitor::get_name()
                      << " synchronize exception: " << e.what() << "\n";
            using R = typename std::invoke_result<Functor>::type;
            if constexpr (!std::is_same<R, void>::value) { return R{}; }
        }
    }

    // Enable processing of synchronized commands.
    // To be called after the first init message has been handled.
    auto allow_command_processing() noexcept -> void;

    // Add a message to reactor queue.
    // It fails if the reactor has been stopped or it has not yet been started.
    auto post(network::zeromq::Message&& in, unsigned idx = 0) -> bool;

    // Schedule message execution.
    // It fails if the reactor has been stopped or it has not yet been started.
    auto post_at(
        network::zeromq::Message&& in,
        std::chrono::time_point<std::chrono::system_clock> t_at) -> bool;

    // Check if the caller is the reactor loop.
    bool in_reactor_thread() const noexcept;

    // Returns the value passed on construction.
    auto name() const noexcept { return name_; }

    // Start the reactor thread.
    // It fails if it has been called already.
    // A stopped reactor cannot be started again.
    auto start() -> bool;

    // Terminate reactor loop.
    // It fails if it has been called already.
    // A stopped reactor cannot be started again.
    auto stop() -> bool;

protected:
    // Handle a message in a subclass.
    virtual auto handle(
        network::zeromq::Message&& in,
        unsigned idx = 0) noexcept -> void = 0;

    // Force a round of queue checks.
    // TODO consider making private. Actor probably does not need to call this
    // explicitly - subject to successful testing outcome.
    auto notify() -> void;

    // Submit a message for deferred handling.
    auto defer(network::zeromq::Message&& in) -> void;

    // Handle all deferred messages in the reactor thread, blocking until
    // completion.
    auto flush_cache() -> void;

    Reactor(std::string name, unsigned qcount = 1) noexcept;

    ~Reactor() override;

    // **These functions are for diagnostic use**

    std::thread::id processing_thread_id();

    // We use it when an operation like message processing exceeds a time limit.
    // It is a subclass' responsibility to identify the last job.
    virtual auto last_job_str() const noexcept -> std::string = 0;

private:
    using Promises = std::queue<std::promise<void>>;
    std::atomic<bool> active_;
    std::atomic<bool> can_process_commands_;
    Promises deferred_promises_;
    mutable std::mutex mtx_queue_state;
    mutable std::condition_variable cv_queue_state;
    std::unique_ptr<std::thread> thread_;

    using CommandQueue = std::queue<std::unique_ptr<HiPCommand>>;
    using MessageQueue = std::vector<std::queue<network::zeromq::Message>>;
    using SchedulerQueue = std::priority_queue<
        Timed<network::zeromq::Message, std::chrono::system_clock>,
        std::vector<Timed<network::zeromq::Message, std::chrono::system_clock>>,
        TimedBefore<network::zeromq::Message, std::chrono::system_clock>>;

    CommandQueue command_queue_;
    MessageQueue message_queue_;
    MessageQueue deferred_queue_;
    SchedulerQueue scheduler_queue_;
    std::optional<std::chrono::time_point<std::chrono::system_clock>> deadline_;
    std::string name_;
    std::thread::id processing_thread_id_;

private:
    auto dequeue_message()
        -> std::optional<std::pair<network::zeromq::Message, unsigned>>;

    auto dequeue_scheduled_message()
        -> std::optional<std::pair<network::zeromq::Message, unsigned>>;

    auto dequeue_deferred() -> std::optional<network::zeromq::Message>;

    auto dequeue_command() noexcept -> std::unique_ptr<HiPCommand>;

    auto process_deferred() -> void;

    auto process_command(std::unique_ptr<HiPCommand>&&) -> void;
    auto process_message(network::zeromq::Message&&, int) -> void;
    auto process_scheduled(network::zeromq::Message&&) -> void;

    auto reactor_loop() noexcept -> int;

    auto reactor_loop_raw() -> int;
    auto time_to_deadline() const -> std::chrono::milliseconds;
};
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

#endif  // UTIL_REACTOR_HPP_
