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
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/detail/buffer_traits.hpp>
#include <boost/beast/core/detail/buffers_range_adaptor.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/impl/async_base.hpp>
#include <boost/beast/core/impl/basic_stream.hpp>
#include <boost/beast/core/impl/buffers_cat.hpp>
#include <boost/beast/core/impl/buffers_prefix.hpp>
#include <boost/beast/core/impl/buffers_suffix.hpp>
#include <boost/beast/core/impl/error.ipp>
#include <boost/beast/core/impl/flat_buffer.hpp>
#include <boost/beast/core/impl/read_size.hpp>
#include <boost/beast/core/impl/string.ipp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/core/string_type.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/detail/basic_parsed_list.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/basic_parser.hpp>
#include <boost/beast/http/impl/basic_parser.ipp>
#include <boost/beast/http/impl/fields.hpp>
#include <boost/beast/http/impl/message.hpp>
#include <boost/beast/http/impl/read.hpp>
#include <boost/beast/http/impl/serializer.hpp>
#include <boost/beast/http/impl/verb.ipp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
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
#include <boost/system/error_code.hpp>
#include <boost/thread/thread.hpp>
#include <boost/type_index/type_index_facade.hpp>
#include <boost/utility/string_view.hpp>
#include <openssl/err.h>
#include <openssl/ssl3.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
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
namespace ssl = boost::asio::ssl;
namespace zmq = opentxs::network::zeromq;

#define IMP "opentxs::api::network::Asio::Imp::"

#pragma GCC diagnostic ignored "-Wold-style-cast"
namespace opentxs::api::network
{
struct Asio::Imp final : public api::network::internal::Asio,
                         public opentxs::internal::StateMachine {
    using Buffer = boost::beast::flat_buffer;
    using Request = http::request<http::empty_body>;
    using Response = http::response<http::string_body>;
    using Resolver = boost::asio::ip::tcp::resolver;
    using SSLContext = boost::asio::ssl::context;
    using SSLStream = boost::beast::ssl_stream<boost::beast::tcp_stream>;
    using Stream = boost::beast::tcp_stream;
    using Type = opentxs::network::asio::Endpoint::Type;

    auto Endpoint() const noexcept -> const char* { return endpoint_.c_str(); }

    auto Connect(const ReadView id, internal::Asio::Socket& socket) noexcept
        -> bool final
    {
        auto lock = sLock{lock_};

        if (shutdown()) { return false; }

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

        if (shutdown()) { return; }

        {
            const auto listen = socket_->Start(endpoint_);

            OT_ASSERT(listen);
        }
        {
            const auto threads = std::thread::hardware_concurrency();

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

        if (shutdown()) { return false; }

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

        if (shutdown()) { return false; }

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

        if (shutdown()) { return output; }

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
        Stop().get();

        {
            auto lock = eLock{lock_};
            work_ = boost::asio::any_io_executor{};
            thread_pool_.join_all();
            context_.stop();
            socket_->Close();
        }
    }

    Imp(const zmq::Context& zmq) noexcept
        : StateMachine([&] { return state_machine(); })
        , zmq_(zmq)
        , endpoint_(zmq_.BuildEndpoint("asio/register", -1, 1))
        , cb_(zmq::ListenCallback::Factory([this](auto& in) { callback(in); }))
        , socket_(zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind))
        , context_()
        , work_(boost::asio::require(
              context_.get_executor(),
              boost::asio::execution::outstanding_work.tracked))
        , thread_pool_()
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

    enum class ResponseType { IPvonly, AddressOnly };
    enum class IPversion { IPV4, IPV6 };

    struct Site {
        const std::string host;
        const std::string service;
        const std::string target;
        const ResponseType response_type;
        const IPversion protocol;
        const unsigned http_version;
    };

    const std::vector<Site> sites{
        {"ip4only.me",
         "http",
         "/api/",
         ResponseType::IPvonly,
         IPversion::IPV4,
         11},
        {"ip6only.me",
         "http",
         "/api/",
         ResponseType::IPvonly,
         IPversion::IPV6,
         11},
        {"ip4.seeip.org",
         "https",
         "/",
         ResponseType::AddressOnly,
         IPversion::IPV4,
         11},
        {"ip6.seeip.org",
         "https",
         "/",
         ResponseType::AddressOnly,
         IPversion::IPV6,
         11}};

    const zmq::Context& zmq_;
    const std::string endpoint_;
    const OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    boost::asio::io_context context_;
    boost::asio::any_io_executor work_;
    boost::thread_group thread_pool_;
    Buffers buffers_;
    mutable std::shared_mutex lock_;
    std::promise<OTData> ipv4_promise_;
    std::promise<OTData> ipv6_promise_;
    std::shared_future<OTData> ipv4_future_;
    std::shared_future<OTData> ipv6_future_;

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
            LogOutput(IMP)(__FUNCTION__)(" : ")(e.what()).Flush();
        }
    }

    void load_root_certificates(
        ssl::context& ctx,
        boost::system::error_code& ec)
    {
        // This is the root certificate for seeip.org.
        std::string cert =
            "# ISRG Root X1\n"
            "-----BEGIN CERTIFICATE-----\n"
            "MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
            "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
            "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
            "WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
            "ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
            "MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
            "h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
            "0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
            "A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
            "T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
            "B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
            "B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
            "KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
            "OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
            "jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
            "qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
            "rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
            "HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
            "hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
            "ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
            "3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
            "NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
            "ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
            "TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
            "jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
            "oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
            "4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
            "mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
            "emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
            "-----END CERTIFICATE-----\n"
            "\n";

        ctx.add_certificate_authority(
            boost::asio::buffer(cert.data(), cert.size()), ec);
    }

    auto retrieve_address_async(
        const struct Site& site,
        std::promise<OTData>&& promise) -> void
    {
        auto resolver_promise = std::promise<Resolver::results_type>{};
        auto resolver_future = resolver_promise.get_future();

        auto resolver = Resolver{Context()};
        resolver.async_resolve(
            site.host,
            site.service,
            [host{site.host},
             resolver_promise{
                 std::forward<std::promise<Resolver::results_type>&&>(
                     resolver_promise)}](const auto& ec, auto results) mutable {
                if (!ec) {
                    resolver_promise.set_value(results);
                } else {
                    resolver_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to resolve host: " + host +
                            ", Error: " + ec.message())));
                }
            });

        auto resolver_results = Resolver::results_type{};
        try {
            resolver_results = resolver_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto connect_promise = std::promise<void>{};
        auto connect_future = connect_promise.get_future();

        auto stream = Stream{Context()};
        stream.expires_after(std::chrono::seconds(10));
        stream.async_connect(
            resolver_results,
            [host{site.host},
             connect_promise{
                 std::forward<std::promise<void>&&>(connect_promise)}](
                const auto& ec, [[maybe_unused]] auto endpoint) mutable {
                if (!ec) {
                    connect_promise.set_value();
                } else {
                    if (beast::error::timeout == ec) {
                        connect_promise.set_exception(
                            std::make_exception_ptr(std::runtime_error(
                                "Timed out connecting to host: " + host)));
                    } else {
                        connect_promise.set_exception(
                            std::make_exception_ptr(std::runtime_error(
                                "Failed to connect to host: " + host +
                                ", Error: " + ec.message())));
                    }
                }
            });

        try {
            connect_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto request_promise = std::promise<void>{};
        auto request_future = request_promise.get_future();

        auto request = Request{http::verb::get, site.target, site.http_version};
        request.set(http::field::host, site.host);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        http::async_write(
            stream,
            request,
            [host{site.host},
             request_promise{
                 std::forward<std::promise<void>&&>(request_promise)}](
                const auto& ec,
                [[maybe_unused]] auto bytes_transferred) mutable {
                if (!ec) {
                    request_promise.set_value();
                } else {
                    request_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to send GET request to host: " + host +
                            ", Error: " + ec.message())));
                }
            });

        try {
            request_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto response_promise = std::promise<void>{};
        auto response_future = response_promise.get_future();

        auto buffer = Buffer{};
        auto response = Response{};
        http::async_read(
            stream,
            buffer,
            response,
            [host{site.host},
             response_promise{
                 std::forward<std::promise<void>&&>(response_promise)}](
                const auto& ec,
                [[maybe_unused]] auto bytes_transferred) mutable {
                if (!ec) {
                    response_promise.set_value();
                } else {
                    response_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to receive GET response from host: " +
                            host + ", Error: " + ec.message())));
                }
            });

        try {
            response_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto response_string = response.body();

        std::string address_string{};
        switch (site.response_type) {
            case ResponseType::IPvonly: {
                auto parts = std::vector<std::string>{};
                algo::split(parts, response_string, algo::is_any_of(","));

                if (parts.size() > 1) { address_string = parts[1]; }
            } break;
            case ResponseType::AddressOnly: {
                address_string = response_string;
            } break;
            default: {
                promise.set_exception(std::make_exception_ptr(
                    std::runtime_error("Unknown response type.")));
                return;
            }
        }

        if (!address_string.empty()) {
            beast::error_code address_ec;
            const auto address = ip::make_address(address_string, address_ec);

            if (!address_ec) {
                LogVerbose(IMP)(__FUNCTION__)(
                    " GET response: IP address: ")(address_string)
                    .Flush();
                if (address.is_v4()) {
                    const auto bytes = address.to_v4().to_bytes();
                    promise.set_value(
                        Data::Factory(bytes.data(), bytes.size()));
                } else if (address.is_v6()) {
                    const auto bytes = address.to_v6().to_bytes();
                    promise.set_value(
                        Data::Factory(bytes.data(), bytes.size()));
                }
            } else {
                promise.set_exception(
                    std::make_exception_ptr(std::runtime_error(
                        "GET response from host: " + site.host +
                        ": invalid IP address: " +
                        address_string.substr(0, 39) +
                        ", Error: " + address_ec.message())));
                return;
            }
        }
    }

    auto retrieve_address_async_ssl(
        const struct Site& site,
        std::promise<OTData>&& promise) -> void
    {
        auto resolver_promise = std::promise<Resolver::results_type>{};
        auto resolver_future = resolver_promise.get_future();

        auto resolver = Resolver{Context()};
        resolver.async_resolve(
            site.host,
            site.service,
            [host{site.host},
             resolver_promise{
                 std::forward<std::promise<Resolver::results_type>&&>(
                     resolver_promise)}](const auto& ec, auto results) mutable {
                if (!ec) {
                    resolver_promise.set_value(results);
                } else {
                    resolver_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to resolve host: " + host +
                            ", Error: " + ec.message())));
                }
            });

        auto resolver_results = Resolver::results_type{};
        try {
            resolver_results = resolver_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        ssl::context ssl_context{ssl::context::tlsv12_client};

        boost::system::error_code ec;
        load_root_certificates(ssl_context, ec);
        if (ec) {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(
                "Error loading root certificates, Error: " + ec.message())));
            return;
        }

        ssl_context.set_verify_mode(ssl::verify_peer);

        auto ssl_stream = SSLStream{Context(), ssl_context};

        auto connect_promise = std::promise<void>{};
        auto connect_future = connect_promise.get_future();

        if (!SSL_set_tlsext_host_name(
                ssl_stream.native_handle(), site.host.c_str())) {
            beast::error_code ec{
                static_cast<int>(::ERR_get_error()),
                boost::asio::error::get_ssl_category()};
            promise.set_exception(std::make_exception_ptr(std::runtime_error(
                "Error calling SSL_set_tlsext_host_name for host: " +
                site.host + ", Error: " + ec.message())));
            return;
        }

        beast::get_lowest_layer(ssl_stream)
            .expires_after(std::chrono::seconds(10));

        beast::get_lowest_layer(ssl_stream)
            .async_connect(
                resolver_results,
                [host{site.host},
                 connect_promise{
                     std::forward<std::promise<void>&&>(connect_promise)}](
                    const auto& ec, [[maybe_unused]] auto endpoint) mutable {
                    if (!ec) {
                        connect_promise.set_value();
                    } else {
                        if (beast::error::timeout == ec) {
                            connect_promise.set_exception(
                                std::make_exception_ptr(std::runtime_error(
                                    "Timed out connecting to host: " + host)));
                        } else {
                            connect_promise.set_exception(
                                std::make_exception_ptr(std::runtime_error(
                                    "Failed to connect to host: " + host +
                                    ", Error: " + ec.message())));
                        }
                    }
                });

        try {
            connect_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto handshake_promise = std::promise<void>{};
        auto handshake_future = handshake_promise.get_future();

        ssl_stream.async_handshake(
            ssl::stream_base::client,
            [host{site.host},
             handshake_promise{std::forward<std::promise<void>&&>(
                 handshake_promise)}](const auto& ec) mutable {
                if (!ec) {
                    handshake_promise.set_value();
                } else {
                    handshake_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Handshake failed to host: " + host +
                            ", Error: " + ec.message())));
                }
            });

        try {
            handshake_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto request_promise = std::promise<void>{};
        auto request_future = request_promise.get_future();

        auto request = Request{http::verb::get, site.target, site.http_version};
        request.set(http::field::host, site.host);
        request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        http::async_write(
            ssl_stream,
            request,
            [host{site.host},
             request_promise{
                 std::forward<std::promise<void>&&>(request_promise)}](
                const auto& ec,
                [[maybe_unused]] auto bytes_transferred) mutable {
                if (!ec) {
                    request_promise.set_value();
                } else {
                    request_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to send GET request to host: " + host +
                            ", Error: " + ec.message())));
                }
            });

        try {
            request_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto response_promise = std::promise<void>{};
        auto response_future = response_promise.get_future();

        auto buffer = Buffer{};
        auto response = Response{};
        http::async_read(
            ssl_stream,
            buffer,
            response,
            [host{site.host},
             response_promise{
                 std::forward<std::promise<void>&&>(response_promise)}](
                const auto& ec,
                [[maybe_unused]] auto bytes_transferred) mutable {
                if (!ec) {
                    response_promise.set_value();
                } else {
                    response_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Failed to receive GET response from  host: " +
                            host + ", Error: " + ec.message())));
                }
            });

        try {
            response_future.get();
        } catch (...) {
            promise.set_exception(std::current_exception());
            return;
        }

        auto response_string = response.body();

        std::string address_string{};
        switch (site.response_type) {
            case ResponseType::IPvonly: {
                auto parts = std::vector<std::string>{};
                algo::split(parts, response_string, algo::is_any_of(","));

                if (parts.size() > 1) { address_string = parts[1]; }
            } break;
            case ResponseType::AddressOnly: {
                address_string = response_string;
            } break;
            default: {
                promise.set_exception(std::make_exception_ptr(
                    std::runtime_error("Unknown response type.")));
                return;
            }
        }

        if (!address_string.empty()) {
            beast::error_code address_ec;
            const auto address = ip::make_address(address_string, address_ec);

            if (!address_ec) {
                LogVerbose(IMP)(__FUNCTION__)(
                    " GET response: IP address: ")(address_string)
                    .Flush();
                if (address.is_v4()) {
                    const auto bytes = address.to_v4().to_bytes();
                    promise.set_value(
                        Data::Factory(bytes.data(), bytes.size()));
                } else if (address.is_v6()) {
                    const auto bytes = address.to_v6().to_bytes();
                    promise.set_value(
                        Data::Factory(bytes.data(), bytes.size()));
                }
            } else {
                promise.set_exception(
                    std::make_exception_ptr(std::runtime_error(
                        "GET response from host: " + site.host +
                        ": invalid IP address: " +
                        address_string.substr(0, 39) +
                        ", Error: " + address_ec.message())));
                return;
            }
        }

        auto shutdown_promise = std::promise<void>{};
        auto shutdown_future = shutdown_promise.get_future();

        ssl_stream.async_shutdown(
            [shutdown_promise{std::forward<std::promise<void>&&>(
                shutdown_promise)}]([[maybe_unused]] const auto& ec) mutable {
                // async_shutdown can return errors depending on the behavior of
                // the server, so log the error but continue.
                // https://stackoverflow.com/questions/25587403/
                // boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                if (!ec) {
                    shutdown_promise.set_value();
                } else {
                    shutdown_promise.set_exception(
                        std::make_exception_ptr(std::runtime_error(
                            "Problem shutting down ssl stream, Error: " +
                            ec.message() + ", continuing execution.")));
                }
            });
        try {
            shutdown_future.get();
        } catch (const std::exception& e) {
            // The value of promise is already set, so log the shutdown error
            // and continue.
            LogVerbose(IMP)(__FUNCTION__)(" ")(e.what()).Flush();
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

        auto promises4 = std::vector<std::promise<OTData>>{};
        auto futures4 = std::vector<std::future<OTData>>{};

        auto promises6 = std::vector<std::promise<OTData>>{};
        auto futures6 = std::vector<std::future<OTData>>{};

        for (const auto& site : sites) {
            if (IPversion::IPV4 == site.protocol) {
                auto& promise4 = promises4.emplace_back();
                futures4.emplace_back(promise4.get_future());

                if ("https" == site.service) {
                    retrieve_address_async_ssl(site, std::move(promise4));
                } else {
                    retrieve_address_async(site, std::move(promise4));
                }
            } else {
                auto& promise6 = promises6.emplace_back();
                futures6.emplace_back(promise6.get_future());

                if ("https" == site.service) {
                    retrieve_address_async_ssl(site, std::move(promise6));
                } else {
                    retrieve_address_async(site, std::move(promise6));
                }
            }
        }

        auto result4 = Data::Factory();
        auto result6 = Data::Factory();

        for (auto& future : futures4) {
            try {
                auto result = future.get();

                if (result->empty()) { continue; }

                result4 = std::move(result);
                break;
            } catch (...) {
                try {
                    auto eptr = std::current_exception();

                    if (eptr) { std::rethrow_exception(eptr); }
                } catch (const std::exception& e) {
                    LogVerbose(IMP)(__FUNCTION__)(" ")(e.what()).Flush();
                }
            }
        }

        for (auto& future : futures6) {
            try {
                auto result = future.get();

                if (result->empty()) { continue; }

                result6 = std::move(result);
                break;
            } catch (...) {
                try {
                    auto eptr = std::current_exception();

                    if (eptr) { std::rethrow_exception(eptr); }
                } catch (const std::exception& e) {
                    LogVerbose(IMP)(__FUNCTION__)(" ")(e.what()).Flush();
                }
            }
        }

        if (result4->empty() && result6->empty()) { again = true; }

        {
            auto lock = eLock{lock_};
            ipv4_promise_.set_value(std::move(result4));
            ipv6_promise_.set_value(std::move(result6));
        }

        return again;
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    Imp& operator=(const Imp&) = delete;
    Imp& operator=(Imp&&) = delete;
};
}  // namespace opentxs::api::network
