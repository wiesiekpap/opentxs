// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <cs_plain_guarded.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string_view>
#include <utility>

//#include "internal/api/network/Asio.hpp"
//#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
//#include "internal/network/zeromq/socket/Pipeline.hpp"
//#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
//#include "internal/util/Timer.hpp"
//#include "opentxs/api/network/Asio.hpp"
//#include "opentxs/api/network/Network.hpp"
//#include "opentxs/api/session/Client.hpp"
//#include "opentxs/api/session/Endpoints.hpp"
//#include "opentxs/api/session/Factory.hpp"
//#include "opentxs/api/session/Session.hpp"
//#include "opentxs/network/zeromq/Context.hpp"
//#include "opentxs/network/zeromq/Pipeline.hpp"
//#include "opentxs/network/zeromq/message/Frame.hpp"
//#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocated.hpp"
//#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
//#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
//#include "util/Work.hpp"
#include "util/threadutil.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{

namespace api
{
class Session;
}  // namespace api

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{

struct Command {
    Command() = default;
    virtual ~Command() {}
    virtual void exec() = 0;
};

template <typename F>
struct SynchronizableCommand : Command {
    SynchronizableCommand(F f)
        : Command{}
        , f_{f}
        , promise_{}
    {
    }
    F f_;
    using R = typename std::invoke_result<F>::type;
    std::promise<R> promise_;
    void exec() override
    {
        if constexpr (std::is_same<R, void>::value) {
            f_();
            promise_.set_value();
        } else {
            promise_.set_value(f_());
        }
    };
};

class Reactor : public ThreadDisplay
{
public:
    template <typename F>
    auto synchronize(F&& f)
    {
        if (in_reactor_thread()) { return f(); }
        tdiag(dname(this), "synchronize command");
        std::unique_ptr<SynchronizableCommand<F>> fc(
            new SynchronizableCommand(std::move(f)));
        std::future<typename std::invoke_result<F>::type> ft =
            fc.get()->promise_.get_future();
        {
            auto cq = command_queue_.lock();
            cq->push(std::move(fc));
            queue_event_flag_.notify_one();
        }
        return ft.get();
    }

protected:
    virtual auto handle(network::zeromq::Message&& in) noexcept -> void = 0;

    auto enqueue(network::zeromq::Message&& in) -> bool;

    bool start();

    bool stop();

    bool in_reactor_thread() const noexcept;

    auto notify() -> void;

    auto wait_for_shutdown() const -> void;

private:
    std::atomic<bool> active_;
    mutable std::mutex queue_event_mtx_;
    mutable std::condition_variable queue_event_flag_;
    std::unique_ptr<std::thread> thread_;
    std::promise<void> promise_shutdown_;
    std::shared_future<void> future_shutdown_;

public:
    using Message = network::zeromq::Message;

protected:
    using CommandQueue =
        libguarded::plain_guarded<std::queue<std::unique_ptr<Command>>>;
    using MessageQueue = libguarded::plain_guarded<std::queue<Message>>;
    std::thread::id processing_thread_id();

private:
    CommandQueue command_queue_;
    MessageQueue message_queue_;

protected:
    const Log& log_;
    CString name_;

private:
    auto dequeue_message() -> std::optional<Message>;

    auto dequeue_command() noexcept -> std::unique_ptr<Command>;

    auto try_post(Message&& m) noexcept -> std::optional<Message>;

protected:
    Reactor(const Log& logger, const CString& name) noexcept;
    ~Reactor() override;

private:
    std::thread::id processing_thread_id_;

private:
    int reactor_loop();
};
}  // namespace opentxs
