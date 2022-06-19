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

auto Reactor::enqueue(network::zeromq::Message&& in, unsigned idx) -> bool
{
    if (active_) {
        {
            auto q = message_queue_.lock();
            auto& mq = q->at(idx);
            mq.push(std::move(in));
        }
        queue_event_flag_.notify_one();
        return true;
    }
    return false;
}

auto Reactor::defer(network::zeromq::Message&& in) -> void
{
    auto q = deferred_queue_.lock();
    auto& mq = (*q)[0];
    mq.push(in);
}

auto Reactor::dequeue_message()
    -> std::optional<std::pair<network::zeromq::Message, unsigned>>
{
    auto q = message_queue_.lock();
    for (auto idx = 0u; idx < q->size(); ++idx) {
        auto& mq = (*q)[idx];
        if (!mq.empty()) {
            auto msg = std::make_pair(std::move(mq.front()), idx);
            mq.pop();
            return msg;
        }
    }
    return {std::nullopt};
}

auto Reactor::dequeue_deferred() -> std::optional<network::zeromq::Message>
{
    std::optional<network::zeromq::Message> msg{};
    auto q = deferred_queue_.lock();
    auto& mq = (*q)[0];
    if (!mq.empty()) {
        msg = std::move(mq.front());
        mq.pop();
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

auto Reactor::try_post(network::zeromq::Message&& m, unsigned idx) noexcept
    -> std::optional<std::pair<network::zeromq::Message, unsigned>>
{
    if (in_reactor_thread()) {
        auto reject =
            std::make_optional<std::pair<network::zeromq::Message, unsigned>>(
                {std::move(m), idx});
        return reject;
    }
    {
        auto q = message_queue_.lock();
        auto& mq = q->at(idx);
        mq.push(m);
    }
    queue_event_flag_.notify_one();
    return {};
}

auto Reactor::process_deferred() -> void
{
    while (active_) {
        auto m = dequeue_deferred();
        if (!m.has_value()) { break; }
        time_it(
            [&] { handle(std::move(m.value())); },
            "XX DEFERRED MESSAGE HANDLER",
            500);
    }
}

auto Reactor::flush_cache() -> void
{
    if (in_reactor_thread()) {
        process_deferred();
    } else {
        std::promise<void> p{};
        auto f = p.get_future();
        {
            auto deferred_promises_handle = deferred_promises_.lock();
            deferred_promises_handle->push_back(std::move(p));
        }
        notify();
        f.get();
    }
}

bool Reactor::start()
{
    tdiag("starting");
    if (active_.exchange(true)) return false;

    thread_.reset(new std::thread(&Reactor::reactor_loop, this));

    return true;
}

bool Reactor::stop()
{
    auto old = active_.exchange(false);
    active_ = false;
    queue_event_flag_.notify_one();
    if (thread_ && !in_reactor_thread() && thread_->joinable()) {
        thread_->join();
    }
    return old;
}

auto Reactor::notify() -> void { queue_event_flag_.notify_one(); }

auto Reactor::wait_for_loop_exit() const -> void
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

Reactor::Reactor(
    const Log& logger,
    const CString& name,
    unsigned qcount) noexcept
    : ThreadDisplay()
    , active_{}
    , deferred_promises_{}
    , queue_event_mtx_{}
    , queue_event_flag_{}
    , thread_{}
    , promise_shutdown_{}
    , future_shutdown_{promise_shutdown_.get_future()}
    , command_queue_{}
    , message_queue_(qcount)
    , deferred_queue_(1)
    , log_{logger}
    , name_{name}
    , processing_thread_id_{}
{
    log_(name_)(" ")(__FUNCTION__).Flush();
    tdiag("Reactor::Reactor");
    assert(message_queue_.lock()->at(0).empty());
    queue_event_flag_.notify_one();
}

Reactor::~Reactor()
{
    tdiag("Reactor::~Reactor 1", processing_thread_id_);
    try {

        if (thread_ && !in_reactor_thread() && thread_->joinable()) {
            active_ = false;
            queue_event_flag_.notify_one();
            thread_->join();
        }
    } catch (const std::exception&) {
        // deliberately silent
    }
    tdiag("Reactor::~Reactor 2", processing_thread_id_);
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
                tdiag("leaving reactor.1 ", processing_thread_id_);
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
            tdiag("reactor initialization complete", processing_thread_id_);
        }

        while (active_) {
            {
                auto deferred_promises_handle = deferred_promises_.lock();
                if (!deferred_promises_handle->empty()) {
                    process_deferred();
                    deferred_promises_handle->front().set_value();
                    deferred_promises_handle->pop_front();
                }
            }

            while (active_) {
                auto c = dequeue_command();
                if (c) {
                    tdiag(typeid(this), "dequeue command");
                    time_it([&] { c->exec(); }, "XX COMMAND EXEC", 1200);
                }

                auto m = dequeue_message();
                if (!active_ || !m.has_value()) { break; }
                auto& [msg, idx] = m.value();
                time_it(
                    [&, m = msg, id = idx]() mutable {
                        handle(std::move(m), id);
                    },
                    "XX MESSAGE HANDLER",
                    1200);
                tadiag("Last job: ", last_job_str());
            }
            if (!active_) { break; }
            std::unique_lock<std::mutex> lck(queue_event_mtx_);
            time_it(
                [&] { queue_event_flag_.wait(lck); }, "XX NOTIFICATION", 20000);
        }
        break;
    }
    tdiag("leaving reactor.2 ", processing_thread_id_);
    promise_shutdown_.set_value();
    return 0;
}

}  // namespace opentxs
