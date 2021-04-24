// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "internal/api/network/Network.hpp"
#include "network/asio/Endpoint.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // iwyu pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"

namespace zmq = opentxs::network::zeromq;

#define IMP "opentxs::api::network::Asio::Imp::"

namespace opentxs::api::network
{
struct Asio::Imp final : public api::network::internal::Asio {
    auto Endpoint() const noexcept -> const char* { return endpoint_.c_str(); }

    auto Connect(const ReadView id, internal::Asio::Socket& socket) noexcept
        -> bool final
    {
        auto lock = sLock{lock_};

        if (false == running_) { return false; }

        if (0 == id.size()) { return false; }

        const auto& endpoint = socket.endpoint_;
        const auto& internal = endpoint.GetInternal().data_;
        socket.socket_.async_connect(
            internal,
            [this, connection{space(id)}, address{endpoint.str()}](
                const auto& e) {
                if (e) {
                    LogVerbose(IMP)(__FUNCTION__)(": asio connect error: ")(
                        e.message())
                        .Flush();
                }

                auto work = zmq_.TaggedReply(
                    reader(connection),
                    e ? WorkType::AsioDisconnect : WorkType::AsioConnect);
                work->AddFrame(address);
                socket_->Send(std::move(work));
            });

        return true;
    }
    auto Context() noexcept -> boost::asio::io_context& final
    {
        return context_;
    }
    auto Init() noexcept -> void
    {
        auto lock = eLock{lock_};

        if (false == running_) { return; }

        {
            const auto listen = socket_->Start(endpoint_);

            OT_ASSERT(listen);
        }
        {
            const auto threads = std::min<unsigned int>(
                2u, std::thread::hardware_concurrency() / 4u);

            for (unsigned int i{0}; i < threads; ++i) {
                auto* thread = thread_pool_.create_thread(
                    boost::bind(&boost::asio::io_context::run, &context_));

                OT_ASSERT(nullptr != thread);
            }
        }
    }
    auto Post(Callback cb) noexcept -> bool final
    {
        auto lock = sLock{lock_};

        if (false == running_) { return false; }

        boost::asio::post(context_, std::move(cb));

        return true;
    }
    auto Receive(
        const ReadView id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        internal::Asio::Socket& socket) noexcept -> bool final
    {
        auto lock = sLock{lock_};

        if (false == running_) { return false; }

        if (0 == id.size()) { return false; }

        auto buf = buffers_.get(bytes);
        auto asioBuffer =
            boost::asio::buffer(buf.second.data(), buf.second.size());
        const auto& endpoint = socket.endpoint_;
        boost::asio::async_read(
            socket.socket_,
            asioBuffer,
            [this,
             connection{space(id)},
             type,
             buffer{std::move(buf)},
             address{endpoint.str()}](const auto& e, auto size) {
                auto work = zmq_.TaggedReply(
                    reader(connection),
                    e ? value(WorkType::AsioDisconnect) : type);

                if (e) {
                    LogVerbose(IMP)(__FUNCTION__)(": asio receive error: ")(
                        e.message())
                        .Flush();
                    work->AddFrame(address);
                } else {
                    work->AddFrame(buffer.second);
                }

                OT_ASSERT(1 < work->Body().size());

                socket_->Send(std::move(work));
                buffers_.clear(buffer.first);
            });

        return true;
    }
    auto Resolve(std::string_view server, std::uint16_t port) const noexcept
        -> Resolved
    {
        auto output = Resolved{};
        auto lock = sLock{lock_};

        if (false == running_) { return output; }

        using Resolver = boost::asio::ip::tcp::resolver;
        using Type = opentxs::network::asio::Endpoint::Type;

        try {
            auto resolver = Resolver{const_cast<Imp*>(this)->context_};
            const auto results = resolver.resolve(
                server, std::to_string(port), Resolver::query::numeric_service);
            output.reserve(results.size());

            for (const auto& result : results) {
                const auto address = result.endpoint().address();

                if (address.is_v4()) {
                    const auto bytes = address.to_v4().to_bytes();
                    output.emplace_back(
                        Type::ipv4,
                        ReadView{
                            reinterpret_cast<const char*>(bytes.data()),
                            bytes.size()},
                        port);
                } else {
                    const auto bytes = address.to_v6().to_bytes();
                    output.emplace_back(
                        Type::ipv6,
                        ReadView{
                            reinterpret_cast<const char*>(bytes.data()),
                            bytes.size()},
                        port);
                }
            }
        } catch (const std::exception& e) {
            LogVerbose(IMP)(__FUNCTION__)(": ")(e.what()).Flush();
        }

        return output;
    }
    auto Shutdown() noexcept -> void
    {
        auto lock = eLock{lock_};

        if (running_) {
            running_ = false;
            context_.stop();
            thread_pool_.join_all();
            work_.reset();
            socket_->Close();
        }
    }

    Imp(const zmq::Context& zmq) noexcept
        : zmq_(zmq)
        , endpoint_(zmq_.BuildEndpoint("asio/register", -1, 1))
        , cb_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
        , socket_(zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind))
        , context_()
        , work_(std::make_unique<boost::asio::io_context::work>(context_))
        , thread_pool_()
        , running_(true)
        , buffers_()
        , lock_()
    {
    }

    ~Imp() final { Shutdown(); }

private:
    struct Buffers {
        auto clear(const int id) noexcept -> void
        {
            auto lock = Lock{lock_};
            buffers_.erase(id);
        }
        auto get(const std::size_t bytes) noexcept
            -> std::pair<int, WritableView>
        {
            auto id = ++counter_;
            auto& buffer = buffers_[id];

            return {id, writer(buffer)(bytes)};
        }

    private:
        mutable std::mutex lock_{};
        int counter_{-1};
        std::map<int, Space> buffers_{};
    };

    const zmq::Context& zmq_;
    const std::string endpoint_;
    const OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    boost::asio::io_context context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    boost::thread_group thread_pool_;
    bool running_;
    Buffers buffers_;
    mutable std::shared_mutex lock_;

    auto callback(zmq::Message& in) noexcept -> void
    {
        const auto header = in.Header();

        if (0 == header.size()) { return; }

        const auto& connectionID = header.at(header.size() - 1u);

        if (0 == connectionID.size()) { return; }

        const auto body = in.Body();

        if (0 == body.size()) { return; }

        try {
            const auto work = [&] {
                try {

                    return body.at(0).as<OTZMQWorkType>();
                } catch (...) {

                    throw std::runtime_error{"Wrong size for work frame"};
                }
            }();

            switch (work) {
                case value(WorkType::AsioRegister): {
                    auto reply = zmq_.TaggedReply(in, WorkType::AsioRegister);
                    reply->AddFrame(connectionID);
                    socket_->Send(reply);
                } break;
                default: {
                    throw std::runtime_error{
                        "Unknown work type " + std::to_string(work)};
                }
            }
        } catch (const std::exception& e) {
            LogOutput(IMP)(__FUNCTION__)(": ")(e.what()).Flush();
        }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::api::network
