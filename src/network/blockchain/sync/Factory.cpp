// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Base.hpp"  // IWYU pragma: associated

#include <iterator>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "Proto.tpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/Query.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/BlockchainP2PChainState.pb.h"
#include "opentxs/protobuf/BlockchainP2PHello.pb.h"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PHello.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PSync.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::network::blockchain::sync
{
auto Factory(const api::Core& api, const zeromq::Message& in) noexcept
    -> std::unique_ptr<Base>
{
    const auto b = in.Body();

    try {
        if (0 >= b.size()) { throw std::runtime_error{"Insufficient frames"}; }

        const auto& typeFrame = b.at(0);
        const auto type = [&] {
            try {

                return typeFrame.as<WorkType>();
            } catch (...) {

                throw std::runtime_error{"Invalid type"};
            }
        }();

        if (WorkType::SyncQuery == type) { return std::make_unique<Query>(0); }

        if (1 >= b.size()) { throw std::runtime_error{"Insufficient frames"}; }

        const auto& helloFrame = b.at(1);

        const auto hello =
            proto::Factory<proto::BlockchainP2PHello>(helloFrame);

        if (false == proto::Validate(hello, VERBOSE)) {
            throw std::runtime_error{"Invalid hello"};
        }

        auto chains = [&] {
            auto out = std::vector<State>{};

            for (const auto& state : hello.state()) {
                auto hash = [&] {
                    auto out = api.Factory().Data();
                    out->Assign(state.hash().data(), state.hash().size());

                    return out;
                }();
                out.emplace_back(State{
                    static_cast<opentxs::blockchain::Type>(state.chain()),
                    {state.height(), std::move(hash)}});
            }

            return out;
        }();

        switch (type) {
            case WorkType::NewBlock:
            case WorkType::SyncReply: {
                if (3 > b.size()) {
                    throw std::runtime_error{"Insufficient frames"};
                }

                const auto& cfheaderFrame = b.at(2);
                auto data = std::vector<Block>{};
                using Chain = opentxs::blockchain::Type;
                auto chain = std::optional<Chain>{std::nullopt};
                using FilterType = opentxs::blockchain::filter::Type;
                auto filterType = std::optional<FilterType>{std::nullopt};
                using Height = opentxs::blockchain::block::Height;
                auto height = Height{-1};

                for (auto i{std::next(b.begin(), 3)}; i != b.end(); ++i) {
                    const auto sync =
                        proto::Factory<proto::BlockchainP2PSync>(*i);

                    if (false == proto::Validate(sync, VERBOSE)) {
                        throw std::runtime_error{"Invalid sync data"};
                    }

                    const auto incomingChain = static_cast<Chain>(sync.chain());
                    const auto incomingHeight =
                        static_cast<Height>(sync.height());
                    const auto incomingType =
                        static_cast<FilterType>(sync.filter_type());

                    if (chain.has_value()) {
                        if (incomingHeight != ++height) {
                            throw std::runtime_error{
                                "Non-contiguous sync data"};
                        }

                        if (incomingChain != chain.value()) {
                            throw std::runtime_error{"Incorrect chain"};
                        }

                        if (incomingType != filterType.value()) {
                            throw std::runtime_error{"Incorrect filter type"};
                        }
                    } else {
                        chain = incomingChain;
                        height = incomingHeight;
                        filterType = incomingType;
                    }

                    data.emplace_back(sync);
                }

                if (0 == chains.size()) {
                    throw std::runtime_error{"missing state"};
                }

                return std::make_unique<Data>(
                    type,
                    std::move(chains.front()),
                    std::move(data),
                    cfheaderFrame.Bytes());
            }
            case WorkType::SyncAcknowledgement: {

                if (3 > b.size()) {
                    throw std::runtime_error{"Insufficient frames"};
                }

                const auto& endpointFrame = b.at(2);

                return std::make_unique<Acknowledgement>(
                    std::move(chains), std::string{endpointFrame.Bytes()});
            }
            case WorkType::SyncRequest: {

                return std::make_unique<Request>(std::move(chains));
            }
            case WorkType::SyncQuery: {
                OT_FAIL;
            }
            default: {
                throw std::runtime_error{"Unsupported type"};
            }
        }
    } catch (const std::exception& e) {
        LogOutput("opentxs::network::blockchain::sync::Base::")(__func__)(": ")(
            e.what())
            .Flush();

        return std::make_unique<Base>();
    }
}
}  // namespace opentxs::network::blockchain::sync
