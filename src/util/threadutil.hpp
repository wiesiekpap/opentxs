#ifndef UTIL_THREADUTIL_HPP_
#define UTIL_THREADUTIL_HPP_

//
// This is a diagnostic module intended to facilitate debugging threads and
// tracing the origin of messages.
//
// Activating
// ----------
//  The default implementation is inactive, i.e. dummied up.
// Presently, it may only be activated for Linux.
// Export USE_TDIAG=1 to build the active version.
//
// Classes
// -------
// ThreadMonitor - a utility class providing information about the current
// thread and all threads in the system. ThreadDisplay - intended as a base
// class for active classes to be debugged, especially Reactor.
//
// Thread names
// ------------
// ThreadMonitor::add_current_thread() is called to give calling thread a unique
// name. ThreadMonitor::default_name() will make up a name based on the calling
// object, with some digits added if necessary to ensure uniqueness.
//
// Diagnostic output
// -----------------
// - tdiag() may be called from any descendant of Reactor to display relative
// time in ms, calling thread by name, reactor thread by name, caller type and
// up to 2 arbitrary arguments,
// - tadiag() works like tdiag except that its output is conditional on an alert
// flag being latched by e.g. exceeded time limit in a time_it call
// NOTE: the following two functions are not fully omplemented, as they may
// access data for defunct threads.
// - ThreadDisplay::show will send serialized thread information to an any
// output stream,
// - ThreadDisplay::show_all will list existing threads to an output stream,
//
// Message marking
// ---------------
// To mark a message prior to sending, use
//
//  auto marker = MessageMarker("Optional tag text", "opt_thr_name");
//  marker.mark(message);
//
// or, simply
//  MessageMarker("Optional tag text", "opt_thr_name").mark(message);
//
// or just
//  MessageMarker().mark(message);
//
// The shortest form will result in an empty tagging text and either the thread
// name or the hexadecimal representation of the thread id.
//
// It is safe to treat any received message like this:
//
//  MessageMarker marker(message);
//  if (marker) std::cout << marker << "\n";
//
// In the example above, the marker info, if found, is removed from the message.
// It is possible to override a default construction parameter in order to leave
// the marker info in the message intact.
// A standalone function to_string() is
// available to convert a received mark to a string.
//
// Enabling and disabling at run-time
// ----------------------------------
// When compiled with USE_TDIAG=1, diagnostic output is valid by default.
// ThreadDisplay functions enable(Tdiag) may be used to disable, enable, or
// enable in individual threads by means of thread_enable().
//

#include <boost/core/demangle.hpp>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
using namespace std::literals::chrono_literals;

#if defined(__linux) && defined(TDIAG)

using handle_type = std::thread::native_handle_type;

// TODO remove unused fields
struct ThreadHandle {
    ThreadHandle();
    ThreadHandle(
        std::string&& name,
        std::string&& description,
        handle_type handle);
    std::string name_;
    std::string description_;
    handle_type native_;
};

class ThreadMonitor
{
public:
    // Name the caller thread and associate it with the provided description.
    static ThreadHandle add_current_thread(
        std::string&& name,
        std::string&& description);
    // Obtain the caller thread name. For an unnamed thread, the hexadecimal
    // representation of its handle will be returned.
    static std::string get_name();
    // Obtain the thread name. For an unnamed thread, the hexadecimal
    // representation of its handle will be returned.
    static std::string get_name(const ThreadHandle& handle);
    // Obtain the thread name. For an unnamed thread, the hexadecimal
    // representation of its handle will be returned.
    static std::string get_name(handle_type native);
    // Return the given name or an empty string.
    static std::string given_name();
    // Set the name of the thread. It is the caller's responsibility to ensure
    // the uniqueness and correctness.
    static bool set_name(
        std::thread::native_handle_type h,
        std::string_view value);
    // Set the name of the caller thread. It is the caller's responsibility to
    // ensure the uniqueness and correctness.
    static bool set_name(std::string_view value);
    // Arbitrary maximum thread name length...
    static constexpr std::size_t max_name_length();
    // Work out a correct and unique thread name from the full caller object
    // name.
    static std::string default_name(std::string&& Class);
    enum class State { Unknown, Idle, Waiting, Blocked, Deadlocked };
    struct ThreadInfo {
        explicit ThreadInfo(
            handle_type = {},
            State state = State{},
            std::string blocking_resource_str = {},
            std::string blocking_comment_str = {});
        handle_type native_;
        State state_;
        std::string blocking_resource_str_;
        std::string blocking_comment_str_;
    };
    // Get names and other info for all the known threads.
    // At present, this includes defunct threads.
    static std::vector<ThreadInfo> get_snapshot();

private:
    static ThreadMonitor* instance();
    ThreadMonitor();
    ~ThreadMonitor();
    ThreadHandle p_add_current_thread(
        std::string&& name,
        std::string&& description);
    ThreadHandle p_get();
    std::string p_get_name();
    std::string p_get_name(const ThreadHandle& handle);
    std::string p_get_name(handle_type native);
    std::string p_given_name();
    std::vector<ThreadInfo> p_get_snapshot();
    ThreadInfo get_info(const ThreadHandle& handle);

private:
    static std::mutex mutex_;
    static ThreadMonitor* instance_;
    std::map<handle_type, ThreadHandle> handles_;
    std::set<std::string> names_;
    std::map<handle_type, ThreadInfo> thread_info_;
};

struct Mytime {
    Mytime();
    static std::int64_t basecount() noexcept;
    static std::int64_t base;
    std::string str();
};

inline std::string mytime()
{
    Mytime m{};
    return m.str();
}

class ThreadDisplay
{
public:
    ThreadDisplay();
    void set_host_thread(const ThreadHandle& value);
    static void show(const ThreadMonitor::ThreadInfo& thr, std::ostream& os);
    static void show_all(std::ostream& os);

    // Enable, selectively enable or disable tdiag output.
    enum class Tdiag { on, selective, off };
    static void enable(Tdiag) noexcept;
    static void thread_enable() noexcept;
    static void thread_disable() noexcept;

protected:
    // Diagnostic output with one variable text field.
    void tdiag(std::string&& s1) const noexcept;
    // Diagnostic output with up to two variable fields.
    template <typename S1, typename S2>
    void tdiag(S1&& s1, S2&& s2, std::string&& tag = "####") const noexcept
    {
        if (!is_enabled()) { return; }
        if constexpr (std::is_same<
                          typename std::decay<S1>::type,
                          std::type_info>::value) {
            tsdiag(std::move(s1), std::move(s2), std::move(tag));
        } else {
            if constexpr (std::is_same<
                              typename std::decay<S2>::type,
                              std::string>::value) {
                ssdiag(std::move(s1), std::move(s2), std::move(tag));
            } else {
                std::ostringstream oss;
                oss << s2;
                ssdiag(std::move(s1), oss.str(), std::move(tag));
            }
        }
    }
    // Conditional tdiag, triggered by a time_it limit being exceeded. It resets
    // the alert flag upon return.
    void tadiag(
        std::string&& s1,
        std::string&& s2 = "",
        std::string&& tag = "##  ") const noexcept;
    // Conditional tdiag, triggered by a time_it limit being exceeded. It resets
    // the alert flag upon return.
    void tadiag(
        const std::type_info& type,
        std::string&& s2 = "",
        std::string&& tag = "##  ") const noexcept;
    std::string get_class() const noexcept;
    // Functor wrapper to time the execution, conditionally display a diagnostic
    // message and throw an alert switch for tadiag.
    template <typename Functor>
    bool time_it(
        Functor f,
        std::string tag = "",
        std::chrono::milliseconds max_ms = 5000ms)
    {
        auto tstart = std::chrono::system_clock::now();
        f();
        auto tfinish = std::chrono::system_clock::now();
        if (auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                tfinish - tstart);
            ms > max_ms) {
            diag_alert_ = true;
            tdiag(std::move(tag), std::to_string(ms.count()) + " ms");
            return false;
        }
        return true;
    }

    virtual ~ThreadDisplay();

private:
    bool is_enabled() const noexcept;
    void ssdiag(
        std::string&& s1,
        std::string&& s2 = "",
        std::string&& tag = "####") const noexcept;
    void tsdiag(
        const std::type_info& type,
        std::string&& s2 = "",
        std::string&& tag = "####") const noexcept;

private:
    static std::mutex diagmtx;
    std::optional<ThreadHandle> thread_handle_;
    mutable std::atomic<bool> diag_alert_;
    static thread_local bool enabled_for_thread_;
    static Tdiag enabled_;
};

class MessageMarker
{
public:
    explicit MessageMarker(network::zeromq::Message& m, bool remove = true);
    explicit MessageMarker(
        std::string_view tag = {},
        std::string threadname = ThreadMonitor::get_name());
    std::size_t mark(network::zeromq::Message& m);
    operator bool() const noexcept;

private:
    constexpr static std::size_t Text_Size = 16;
    constexpr static std::uint64_t Marker = 0xDEAD00FF;
    friend std::ostream& operator<<(std::ostream&, const MessageMarker&);
    char tag_[Text_Size];
    std::uint32_t check_;
    char name_[Text_Size];
};

std::ostream& operator<<(std::ostream& os, const MessageMarker& m);

std::string to_string(const MessageMarker&);

#else  // defined(__linux) && defined(TDIAG)

using handle_type = std::thread::native_handle_type;
struct ThreadHandle {
    ThreadHandle() {}
    ThreadHandle(std::string&&, std::string&&, handle_type) {}
};

class ThreadMonitor
{
public:
    struct ThreadInfo {
    };
    static ThreadHandle add_current_thread(std::string&&, std::string&&)
    {
        return {};
    }
    static std::string get_name() { return {}; }
    static std::string get_name(const ThreadHandle&) { return {}; }
    static std::string get_name(handle_type native) { return {}; }
    static std::string given_name() { return {}; }
    static std::string default_name() { return {}; }
    static bool set_name(std::thread::native_handle_type, std::string_view)
    {
        return true;
    }
    static bool set_name(std::string_view) { return true; }
    // Arbitrary maximum thread name length...
    static constexpr std::size_t max_name_length() noexcept { return 0; }
    static std::string default_name(std::string&& Class) noexcept { return {}; }
    enum class State { Unknown, Idle, Waiting, Blocked, Deadlocked };
    static std::vector<ThreadInfo> get_snapshot() { return {}; }
};

class ThreadDisplay
{
public:
    ThreadDisplay() = default;
    virtual ~ThreadDisplay() {}
    void set_host_thread(const ThreadHandle&) {}
    static void show(const ThreadMonitor::ThreadInfo& thr, std::ostream& os) {}
    static void show_all(std::ostream& os) {}
    enum class Tdiag { on, selective, off };
    static void enable(Tdiag) noexcept {}
    static void thread_enable() noexcept {}
    static void thread_disable() noexcept {}

protected:
    void tdiag(std::string&&) const noexcept {}
    template <typename S1, typename S2>
    void tdiag(S1&&, S2&&, std::string&& = "") const noexcept
    {
    }
    void tadiag(std::string&&, std::string&& = "", std::string&& = "")
        const noexcept
    {
    }
    void tadiag(const std::type_info&, std::string&& = "", std::string&& = "")
        const noexcept
    {
    }

    std::string get_class() const noexcept { return {}; }
    template <typename Functor>
    static void time_it(
        Functor f,
        std::string = {},
        std::chrono::milliseconds = {})
    {
        f();
    }
};

class MessageMarker
{
public:
    explicit MessageMarker(network::zeromq::Message&, bool = true) {}
    explicit MessageMarker(
        std::string_view = {},
        std::string = ThreadMonitor::get_name())
    {
    }
    std::size_t mark(network::zeromq::Message&) { return 0; }
    operator bool() const noexcept { return false; }
};

inline std::ostream& operator<<(std::ostream& os, const MessageMarker&)
{
    return os;
}

inline std::string to_string(const MessageMarker&) { return {}; }

#endif  // defined(__linux) && defined(TDIAG)

}  // namespace opentxs

#endif  // UTIL_THREADUTIL_HPP_
