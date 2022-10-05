// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/filteroracle/FilterOracle.hpp"  // IWYU pragma: associated

#include "blockchain/node/filteroracle/FilterDownloader.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/node/Manager.hpp"

namespace opentxs::blockchain::node::implementation
{

auto FilterOracle::FilterDownloader::NextBatch() noexcept -> BatchType
{
    return allocate_batch(type_);
}
auto FilterOracle::FilterDownloader::UpdatePosition(const Position& pos) -> void
{
    try {
        auto current = known();
        auto hashes = header_.Ancestors(current, pos, 20000);

        OT_ASSERT(!hashes.empty());

        auto prior = Previous{std::nullopt};
        auto& first = hashes.front();

        if (first != current) {
            auto promise = std::promise<cfilter::Header>{};
            auto cfheader = db_.LoadFilterHeader(type_, first.hash_.Bytes());

            OT_ASSERT(false == cfheader.IsNull());

            promise.set_value(std::move(cfheader));
            prior.emplace(std::move(first), promise.get_future());
        }
        hashes.erase(hashes.begin());
        update_position(std::move(hashes), type_, std::move(prior));
    } catch (...) {
    }
}

FilterOracle::FilterDownloader::FilterDownloader(
    const api::Session& api,
    database::Cfilter& db,
    const HeaderOracle& header,
    const internal::Manager& node,
    const blockchain::Type chain,
    const cfilter::Type type,
    const UnallocatedCString& shutdown,
    const filteroracle::NotifyCallback& notify) noexcept
    : FilterDM(
          [&] { return db.FilterTip(type); }(),
          [&] {
              auto promise = std::promise<cfilter::Header>{};
              const auto tip = db.FilterTip(type);
              promise.set_value(db.LoadFilterHeader(type, tip.hash_.Bytes()));

              return Finished{promise.get_future()};
          }(),
          "cfilter",
          20000,
          10000)
    , FilterWorker(api, "FilterDownloader")
    , db_(db)
    , header_(header)
    , node_(node)
    , chain_(chain)
    , type_(type)
    , notify_(notify)
    , last_job_{}
{
    init_executor({shutdown});
    start();
}

FilterOracle::FilterDownloader::~FilterDownloader()
{
    try {
        signal_shutdown().get();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        // TODO MT-34 improve
    }
}

auto FilterOracle::FilterDownloader::batch_ready() const noexcept -> void
{
    node_.JobReady(PeerManagerJobs::JobAvailableCfilters);
}
auto FilterOracle::FilterDownloader::batch_size(std::size_t in) noexcept
    -> std::size_t
{
    if (in < 10) {

        return 1;
    } else if (in < 100) {

        return 10;
    } else if (in < 1000) {

        return 100;
    } else {

        return 1000;
    }
}
auto FilterOracle::FilterDownloader::check_task(TaskType&) const noexcept
    -> void
{
}
auto FilterOracle::FilterDownloader::trigger_state_machine() const noexcept
    -> void
{
    trigger();
}
auto FilterOracle::FilterDownloader::update_tip(
    const Position& position,
    const cfilter::Header&) const noexcept -> void
{
    const auto saved = db_.SetFilterTip(type_, position);

    OT_ASSERT(saved);

    LogDetail()(print(chain_))(" cfilter chain updated to height ")(
        position.height_)
        .Flush();
    notify_(type_, position);
}

auto FilterOracle::FilterDownloader::process_reset(
    const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(3 < body.size());

    auto position = Position{
        body.at(1).as<block::Height>(), block::Hash{body.at(2).Bytes()}};
    auto promise = std::promise<cfilter::Header>{};
    promise.set_value(body.at(3).Bytes());
    Reset(position, promise.get_future());
}
auto FilterOracle::FilterDownloader::queue_processing(
    DownloadedData&& data) noexcept -> void
{
    if (data.empty()) { return; }

    auto filters = Vector<database::Cfilter::CFilterParams>{};

    for (const auto& task : data) {
        const auto& priorCfheader = task->previous_.get();
        auto& cfilter = const_cast<GCS&>(task->data_.get());
        const auto& block = task->position_.hash_;
        const auto expected = db_.LoadFilterHash(type_, block.Bytes());

        if (expected == cfilter.Hash()) {
            task->process(cfilter.Header(priorCfheader));
            filters.emplace_back(block, std::move(cfilter));
        } else {
            LogError()("Filter for block ")(task->position_)(
                " does not match header. Received: ")(cfilter.Hash().asHex())(
                " expected: ")(expected.asHex())
                .Flush();
            task->redownload();
            break;
        }
    }

    const auto saved = db_.StoreFilters(type_, std::move(filters));

    OT_ASSERT(saved);
}

auto FilterOracle::FilterDownloader::pipeline(zmq::Message&& in) -> void
{
    if (!running_.load()) { return; }

    const auto body = in.Body();

    OT_ASSERT(0 < body.size());

    using Work = FilterOracle::Work;
    const auto work = body.at(0).as<Work>();

    switch (work) {
        case Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case Work::reset_filter_tip: {
            process_reset(in);
        } break;
        case Work::heartbeat: {
            UpdatePosition(db_.FilterHeaderTip(type_));
            run_if_enabled();
        } break;
        case Work::statemachine: {
            run_if_enabled();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto FilterOracle::FilterDownloader::state_machine() noexcept -> int
{
    tdiag("FilterDownloader::state_machine");
    return FilterDM::state_machine() ? 20 : 400;
}

auto FilterOracle::FilterDownloader::shut_down() noexcept -> void
{
    close_pipeline();
    // TODO MT-34 investigate what other actions might be needed
}

auto FilterOracle::FilterDownloader::last_job_str() const noexcept
    -> std::string
{
    return FilterOracle::to_str(last_job_);
}

}  // namespace opentxs::blockchain::node::implementation
