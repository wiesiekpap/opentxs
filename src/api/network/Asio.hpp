// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/detail/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/yes_no_type.hpp>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/detail/buffers_range_adaptor.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/impl/basic_stream.hpp>
#include <boost/beast/core/impl/buffers_cat.hpp>
#include <boost/beast/core/impl/buffers_prefix.hpp>
#include <boost/beast/core/impl/buffers_suffix.hpp>
#include <boost/beast/core/impl/flat_buffer.hpp>
#include <boost/beast/core/impl/multi_buffer.hpp>
#include <boost/beast/core/impl/string.ipp>
#include <boost/beast/core/string_type.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/detail/basic_parsed_list.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/basic_parser.hpp>
#include <boost/beast/http/impl/basic_parser.ipp>
#include <boost/beast/http/impl/fields.hpp>
#include <boost/beast/http/impl/message.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/serializer.hpp>
#include <boost/beast/http/impl/status.ipp>
#include <boost/beast/http/impl/verb.ipp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/version.hpp>
#include <boost/bind/bind.hpp>
#include <boost/core/addressof.hpp>
#include <boost/function/function_base.hpp>
#include <boost/intrusive/detail/list_iterator.hpp>
#include <boost/intrusive/detail/tree_iterator.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/mp11/detail/mp_with_index.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/thread/thread.hpp>
#include <boost/type_index/type_index_facade.hpp>
#include <boost/utility/string_view.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/StateMachine.hpp"
#include "internal/api/network/Network.hpp"
#include "network/asio/Endpoint.hpp"
#include "network/asio/Socket.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/core/Data.hpp"
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

namespace algo = boost::algorithm;
namespace beast = boost::beast;
namespace http = boost::beast::http;
namespace ip = boost::asio::ip;
namespace zmq = opentxs::network::zeromq;

#define IMP "opentxs::api::network::Asio::Imp::"

namespace opentxs::api::network
{
struct Asio::Imp final : public api::network::internal::Asio,
                         public opentxs::internal::StateMachine {
    using Buffer = boost::beast::flat_buffer;
    using Request = http::request<http::string_body>;
    using Response = http::response<http::dynamic_body>;
    using Resolver = boost::asio::ip::tcp::resolver;
    using Stream = boost::beast::tcp_stream;
    using Type = opentxs::network::asio::Endpoint::Type;

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
                    LogVerbose(IMP)(__FUNCTION__)(
                        ": asio connect error: ")(e.message())
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
    auto GetPublicAddress4() const noexcept -> std::shared_future<OTData>
    {
        auto lock = sLock{lock_};

        return ipv4_future_;
    }
    auto GetPublicAddress6() const noexcept -> std::shared_future<OTData>
    {
        auto lock = sLock{lock_};

        return ipv6_future_;
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
    auto Post(Asio::Callback cb) noexcept -> bool final
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

        auto bufData = buffers_.get(bytes);
        const auto& endpoint = socket.endpoint_;
        boost::asio::async_read(
            socket.socket_,
            bufData.second,
            [this,
             connection{space(id)},
             type,
             bufData,
             address{endpoint.str()}](const auto& e, auto size) {
                const auto& [index, buffer] = bufData;
                auto work = zmq_.TaggedReply(
                    reader(connection),
                    e ? value(WorkType::AsioDisconnect) : type);

                if (e) {
                    LogVerbose(IMP)(__FUNCTION__)(
                        ": asio receive error: ")(e.message())
                        .Flush();
                    work->AddFrame(address);
                } else {
                    work->AddFrame(buffer.data(), buffer.size());
                }

                OT_ASSERT(1 < work->Body().size());

                socket_->Send(std::move(work));
                buffers_.clear(index);
            });

        return true;
    }
    auto Resolve(std::string_view server, std::uint16_t port) const noexcept
        -> Resolved
    {
        auto output = Resolved{};
        auto lock = sLock{lock_};

        if (false == running_) { return output; }

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

        Stop().get();
    }

    Imp(const zmq::Context& zmq) noexcept
        : StateMachine([&] { return state_machine(); })
        , zmq_(zmq)
        , endpoint_(zmq_.BuildEndpoint("asio/register", -1, 1))
        , cb_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
        , socket_(zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind))
        , context_()
        , work_(std::make_unique<boost::asio::io_context::work>(context_))
        , thread_pool_()
        , running_(true)
        , buffers_()
        , lock_()
        , ipv4_promise_()
        , ipv6_promise_()
        , ipv4_future_(ipv4_promise_.get_future())
        , ipv6_future_(ipv6_promise_.get_future())
    {
        Trigger();
    }

    ~Imp() final { Shutdown(); }

private:
    struct Buffers {
        using AsioBuffer = decltype(boost::asio::buffer(
            std::declval<void*>(),
            std::declval<std::size_t>()));
        using Index = std::int64_t;

        auto clear(Index id) noexcept -> void
        {
            auto lock = Lock{lock_};
            buffers_.erase(id);
        }
        auto get(const std::size_t bytes) noexcept
            -> std::pair<Index, AsioBuffer>
        {
            auto lock = Lock{lock_};
            auto id = ++counter_;
            auto& buffer = buffers_[id];
            buffer.resize(bytes, std::byte{0x0});

            return {id, boost::asio::buffer(buffer.data(), buffer.size())};
        }

    private:
        mutable std::mutex lock_{};
        Index counter_{-1};
        std::map<Index, Space> buffers_{};
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
    std::promise<OTData> ipv4_promise_;
    std::promise<OTData> ipv6_promise_;
    std::shared_future<OTData> ipv4_future_;
    std::shared_future<OTData> ipv6_future_;
    const std::string ipv4_host_{"ip4only.me"};
    const std::string ipv6_host_{"ip6only.me"};
    const std::string service_{"http"};
    const std::string target_{"/api/"};
    const unsigned version_{11};  // HTTP 1.1

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
    auto retrieve_address(const std::string host) -> OTData
    {
        try {
            auto resolver = Resolver{Context()};
            auto stream = Stream{Context()};

            auto results = resolver.resolve(host, service_);

            stream.connect(results);

            auto request = Request{http::verb::get, target_, version_};
            request.set(http::field::host, host);
            request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            http::write(stream, request);

            auto buffer = Buffer{};
            auto response = Response{};

            http::read(stream, buffer, response);

            auto response_string = std::stringstream{};
            response_string << response;

            auto parts = std::vector<std::string>{};
            algo::split(parts, response_string.str(), algo::is_any_of(","));

            if (parts.size() > 1) {
                const auto address = ip::make_address(parts[2]);

                if (address.is_v4()) {
                    const auto bytes = address.to_v4().to_bytes();
                    return Data::Factory(bytes.data(), bytes.size());
                } else if (address.is_v6()) {
                    const auto bytes = address.to_v6().to_bytes();
                    return Data::Factory(bytes.data(), bytes.size());
                }
            }

            return Data::Factory();
        } catch (const std::exception& e) {
            LogVerbose(IMP)(__FUNCTION__)(": ")(e.what()).Flush();
            return Data::Factory();
        }
    }
    auto state_machine() noexcept -> bool
    {
        auto again{false};

        {
            auto lock = eLock{lock_};
            ipv4_promise_ = {};
            ipv6_promise_ = {};
            ipv4_future_ = ipv4_promise_.get_future();
            ipv6_future_ = ipv6_promise_.get_future();
        }

        auto v4data = retrieve_address(ipv4_host_);
        auto v6data = retrieve_address(ipv6_host_);

        {
            auto lock = eLock{lock_};
            ipv4_promise_.set_value(v4data);
            ipv6_promise_.set_value(v6data);
        }

        if (v4data->empty() && v6data->empty()) { again = true; }

        return again;
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::api::network
