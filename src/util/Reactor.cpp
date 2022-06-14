// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#include "util/Reactor.hpp"

namespace opentxs
{

std::thread::id Reactor::processing_thread_id()
{
    return processing_thread_id_;
}

auto Reactor::enqueue(network::zeromq::Message&& in) -> bool
{
    if (active_) {
        {
            auto mq = message_queue_.lock();
            mq->push(in);
        }
        queue_event_flag_.notify_all();
        return true;
    }
    return false;
}

auto Reactor::dequeue_message() -> std::optional<Message>
{
    std::optional<Message> msg{};
    auto mq = message_queue_.lock();
    if (!mq->empty()) {
        msg = std::move(mq->front());
        mq->pop();
    }
    return msg;
}

auto Reactor::dequeue_command() noexcept -> std::unique_ptr<Command>
{
    auto cq = command_queue_.lock();
    if (!cq->empty()) {
        auto c = std::move(cq->front());
        cq->pop();
        return c;
    }
    return {};
}

auto Reactor::try_post(Message&& m) noexcept -> std::optional<Message>
{
    if (in_reactor_thread()) {
        auto reject(std::move(m));
        return reject;
    }
    {
        auto mq = message_queue_.lock();
        mq->push(m);
    }
    queue_event_flag_.notify_one();
    return {};
}

bool Reactor::start()
{

    if (active_.exchange(true)) return false;

    thread_.reset(new std::thread(&Reactor::reactor_loop, this));

    return true;
}

bool Reactor::stop() { return active_.exchange(false); }

auto Reactor::notify() -> void { queue_event_flag_.notify_one(); }

auto Reactor::wait_for_shutdown() const -> void
{
    try {
        auto f = future_shutdown_;
        f.get();
    } catch (const std::exception& e) {
        log_(name_)(" ")(__FUNCTION__)(": ")(e.what()).Flush();
        throw;
    }
}

bool Reactor::in_reactor_thread() const noexcept
{
    return thread_ && thread_->get_id() == std::this_thread::get_id();
}

Reactor::Reactor(const Log& logger, const CString& name) noexcept
    : ThreadDisplay()
    , active_{}
    , queue_event_mtx_{}
    , queue_event_flag_{}
    , thread_{}
    , promise_shutdown_{}
    , future_shutdown_{promise_shutdown_.get_future()}
    , command_queue_{}
    , message_queue_{}
    , log_{logger}
    , name_{name}
    , processing_thread_id_{}
{
    log_(name_)(" ")(__FUNCTION__).Flush();
    tdiag("Reactor::Reactor");
    queue_event_flag_.notify_one();
}

Reactor::~Reactor()
{
    std::ostringstream os{};
    os << processing_thread_id_;
    tdiag("Reactor::~Reactor 1", os.str());
    try {

        if (thread_ && !in_reactor_thread() && thread_->joinable()) {
            thread_->join();
        }
    } catch (const std::exception&) {
        // deliberately silent
    }
    tdiag("Reactor::~Reactor 2", os.str());
}

int Reactor::reactor_loop()
{
    while (1) {
        if (active_) {
            std::unique_lock<std::mutex> lck(queue_event_mtx_);
            queue_event_flag_.wait(lck);
        }
        if (!active_) {
            {
                std::ostringstream os{};
                os << processing_thread_id_;
                tdiag("leaving reactor", os.str());
            }
            break;
        }

        auto cname = ThreadMonitor::default_name(get_class());
        std::string descr{name_};
        auto thread_handle = ThreadMonitor::add_current_thread(
            std::move(cname), std::move(descr));
        set_host_thread(thread_handle);
        processing_thread_id_ = std::this_thread::get_id();

        {
            std::ostringstream os{};
            os << processing_thread_id_;
            tdiag("reactor initialization", os.str());
        }

        while (active_) {
            while (active_) {
                auto c = dequeue_command();
                if (c) {
                    tdiag(dname(this), "dequeue command");
                    c->exec();
                }

                auto m = dequeue_message();
                if (!active_ || !m.has_value()) { break; }
                handle(std::move(m.value()));
            }
            if (!active_) { break; }
            std::unique_lock<std::mutex> lck(queue_event_mtx_);
            queue_event_flag_.wait(lck);
        }
        break;
    }
    std::ostringstream os{};
    os << processing_thread_id_;
    tdiag("leaving reactor", os.str());
    promise_shutdown_.set_value();
    return 0;
}

}  // namespace opentxs
