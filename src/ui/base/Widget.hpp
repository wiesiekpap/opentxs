// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/ui/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct UI;
}  // namespace internal

class Manager;
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
template <typename T>
auto extract_custom_ptr(
    CustomData& custom,
    const std::size_t index = 0) noexcept -> std::unique_ptr<T>
{
    OT_ASSERT((index + 1) <= custom.size());

    auto& ptr = custom.at(index);
    auto output = std::unique_ptr<T>{static_cast<T*>(ptr)};

    OT_ASSERT(output);

    ptr = nullptr;

    return output;
}

template <typename T>
auto extract_custom(CustomData& custom, const std::size_t index = 0) noexcept
    -> T
{
    auto ptr = extract_custom_ptr<T>(custom, index);
    auto output = T{std::move(*ptr)};

    return output;
}

auto verify_empty(const CustomData& custom) noexcept -> bool;

class Widget : virtual public opentxs::ui::Widget
{
public:
    using Message = network::zeromq::Message;

    const api::client::Manager& api_;

    class MessageFunctor
    {
    public:
        virtual void operator()(Widget* object, const Message&)
            const noexcept = 0;

        virtual ~MessageFunctor() = default;

    protected:
        MessageFunctor() noexcept = default;

    private:
        MessageFunctor(const MessageFunctor&) = delete;
        MessageFunctor(MessageFunctor&&) = delete;
        auto operator=(const MessageFunctor&) -> MessageFunctor& = delete;
        auto operator=(MessageFunctor&&) -> MessageFunctor& = delete;
    };

    template <typename T>
    class MessageProcessor : virtual public MessageFunctor
    {
    public:
        using Function = void (T::*)(const Message&);

        void operator()(Widget* object, const Message& message)
            const noexcept final
        {
            auto real = dynamic_cast<T*>(object);

            OT_ASSERT(nullptr != real)

            (real->*callback_)(message);
        }

        MessageProcessor(Function callback) noexcept
            : callback_(callback)
        {
        }
        MessageProcessor(MessageProcessor&&) = default;
        auto operator=(MessageProcessor&&) -> MessageProcessor& = default;

    private:
        Function callback_;

        MessageProcessor() = delete;
        MessageProcessor(const MessageProcessor&) = delete;
        auto operator=(const MessageProcessor&) -> MessageProcessor& = delete;
    };

    auto ClearCallbacks() const noexcept -> void override
    {
        ui_.ClearUICallbacks(widget_id_);
    }
    auto SetCallback(SimpleCallback cb) const noexcept -> void final
    {
        ui_.RegisterUICallback(WidgetID(), cb);
    }
    auto WidgetID() const noexcept -> OTIdentifier final { return widget_id_; }

    ~Widget() override;

protected:
    using ListenerDefinition = std::pair<std::string, MessageFunctor*>;
    using ListenerDefinitions = std::vector<ListenerDefinition>;

    const OTIdentifier widget_id_;

    virtual void setup_listeners(
        const ListenerDefinitions& definitions) noexcept;
    auto UpdateNotify() const noexcept -> void
    {
        ui_.ActivateUICallback(WidgetID());
    }

    Widget(
        const api::client::Manager& api,
        const Identifier& id,
        const SimpleCallback& cb = {}) noexcept;

private:
    const api::client::internal::UI& ui_;
    std::vector<OTZMQListenCallback> callbacks_;
    std::vector<OTZMQSubscribeSocket> listeners_;

    Widget() = delete;
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
    auto operator=(const Widget&) -> Widget& = delete;
    auto operator=(Widget&&) -> Widget& = delete;
};  // namespace opentxs::ui::implementation
}  // namespace opentxs::ui::implementation
