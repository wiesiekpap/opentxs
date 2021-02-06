// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <map>
#include <set>
#include <thread>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::implementation::Blockchain::ThreadPoolManager::"

namespace opentxs::blockchain::client::internal
{
auto ThreadPool::Capacity() noexcept -> std::size_t
{
    return std::thread::hardware_concurrency();
}

auto ThreadPool::MakeWork(
    const api::Core& api,
    const blockchain::Type chain,
    const Work type) noexcept -> OTZMQMessage
{
    auto work = api.ZeroMQ().Message(chain);
    work->AddFrame(type);
    work->StartBody();

    return work;
}
}  // namespace opentxs::blockchain::client::internal

namespace opentxs::api::client::implementation
{
Blockchain::ThreadPoolManager::ThreadPoolManager(const api::Core& api) noexcept
    : api_(api)
    , map_(init())
    , running_(true)
    , int_(api_.ZeroMQ().PushSocket(zmq::socket::Socket::Direction::Bind))
    , workers_()
    , cbi_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
    , cbe_(zmq::ListenCallback::Factory([this](auto& in) { int_->Send(in); }))
    , ext_(api_.ZeroMQ().PullSocket(cbe_, zmq::socket::Socket::Direction::Bind))
    , init_(false)
    , lock_()
{
}

auto Blockchain::ThreadPoolManager::callback(zmq::Message& in) noexcept -> void
{
    const auto header = in.Header();

    if (2 > header.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    auto worker = run(header.at(0).as<Chain>());

    using Filters = opentxs::blockchain::client::internal::FilterOracle;
    using Wallet = opentxs::blockchain::client::internal::Wallet;

    if (false == worker.has_value()) { return; }

    switch (header.at(1).as<Work>()) {
        case Work::HDAccount: {
            Wallet::ProcessThreadPool(in);
        } break;
        case Work::SyncDataFiltersIncoming: {
            Filters::ProcessThreadPool(in);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::ThreadPoolManager::Endpoint() const noexcept -> std::string
{
    return api_.Endpoints().InternalBlockchainThreadPool();
}

auto Blockchain::ThreadPoolManager::init() noexcept -> NetworkMap
{
    auto output = NetworkMap{};

    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        auto& [active, running, promise, future, mutex] = output[chain];
        active = true;
        future = promise.get_future();
    }

    {
        auto& [active, running, promise, future, mutex] =
            output[opentxs::blockchain::Type::UnitTest];
        active = true;
        future = promise.get_future();
    }

    return output;
}

auto Blockchain::ThreadPoolManager::Reset(const Chain chain) const noexcept
    -> void
{
    if (false == running_.load()) { return; }

    startup();
    auto& [active, running, promise, future, mutex] = map_.at(chain);
    Lock lock(mutex);

    if (false == active) {
        try {
            promise.set_value();
        } catch (...) {
        }

        promise = {};
        future = promise.get_future();
        active = true;
    }
}

auto Blockchain::ThreadPoolManager::run(const Chain chain) noexcept
    -> std::optional<Worker>
{
    auto& data = map_.at(chain);
    auto& [active, running, promise, future, mutex] = data;
    Lock lock(mutex);
    auto output = std::optional<Worker>{};

    if (active) { output.emplace(data); }

    return output;
}

auto Blockchain::ThreadPoolManager::Shutdown() noexcept -> void
{
    if (running_.exchange(false)) {
        ext_->Close();
        int_->Close();
        auto futures = std::vector<std::shared_future<void>>{};

        for (auto& [chain, data] : map_) { futures.emplace_back(Stop(chain)); }

        for (auto& future : futures) { future.get(); }
    }
}

auto Blockchain::ThreadPoolManager::startup() const noexcept -> void
{
    Lock lock(lock_);

    if (init_) { return; }

    const auto target = Capacity();
    workers_.reserve(target);
    const auto random = Identifier::Random();
    const auto endpoint = std::string{"inproc://"} + random->str();
    auto zmq = int_->Start(endpoint);

    OT_ASSERT(zmq);

    for (unsigned int i{0}; i < target; ++i) {
        auto& worker = workers_.emplace_back(api_.ZeroMQ().PullSocket(
            cbi_, zmq::socket::Socket::Direction::Connect));
        auto zmq = worker->Start(endpoint);

        OT_ASSERT(zmq);

        LogTrace("Started blockchain worker thread ")(i).Flush();
    }

    zmq = ext_->Start(Endpoint());

    OT_ASSERT(zmq);

    init_ = true;
}

auto Blockchain::ThreadPoolManager::Stop(const Chain chain) const noexcept
    -> Future
{
    try {
        auto& [active, running, promise, future, mutex] = map_.at(chain);
        Lock lock(mutex);
        active = false;

        if (0 == running) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }

        return future;
    } catch (...) {
        auto promise = std::promise<void>{};
        promise.set_value();

        return promise.get_future();
    }
}

Blockchain::ThreadPoolManager::~ThreadPoolManager()
{
    if (running_.load()) { Shutdown(); }
}
}  // namespace opentxs::api::client::implementation
