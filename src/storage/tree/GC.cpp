// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Root.hpp"  // IWYU pragma: associated

#include <ctime>
#include <utility>

#include "internal/api/network/Network.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/protobuf/StorageRoot.pb.h"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Log.hpp"
#include "storage/tree/Node.hpp"
#include "storage/tree/Tree.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::storage
{
Root::GC::GC(
    const api::network::Asio& asio,
    const Driver& driver,
    const std::int64_t interval) noexcept
    : asio_(asio)
    , driver_(driver)
    , interval_(interval)
    , lock_()
    , running_(Flag::Factory(false))
    , resume_(Flag::Factory(false))
    , root_(Node::BLANK_HASH)
    , last_(static_cast<std::int64_t>(std::time(nullptr)))
    , promise_()
    , future_(promise_.get_future())
{
    promise_.set_value(true);
}

auto Root::GC::Cleanup() noexcept -> void { future_.get(); }

auto Root::GC::Check(const std::string root) noexcept -> CheckState
{
    if (0 == interval_) {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection disabled").Flush();

        return CheckState::Skip;
    }

    if (running_.get()) {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection in progress")
            .Flush();

        return CheckState::Skip;
    }

    const auto run = [this] {
        promise_ = {};
        future_ = promise_.get_future();
        running_->On();
        resume_->On();
    };
    auto lock = Lock{lock_};

    if (resume_.get()) {
        run();

        return CheckState::Resume;
    } else {
        const std::uint64_t time = std::time(nullptr);
        const bool intervalExceeded = ((time - last_.load()) > interval_);

        if (intervalExceeded) {
            run();
            root_ = std::move(root);

            return CheckState::Start;
        } else {

            return CheckState::Skip;
        }
    }
}

auto Root::GC::collect_garbage(
    const bool from,
    const Driver* to,
    const SimpleCallback done) noexcept -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Beginning garbage collection.").Flush();
    auto success{false};
    auto postcondition = ScopeGuard{[&] { promise_.set_value(success); }};
    auto temp = storage::Tree{driver_, root_};
    success = temp.Migrate(*to);

    if (success) {
        driver_.EmptyBucket(from);
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("Garbage collection failed").Flush();
    }

    {
        auto lock = Lock{lock_};
        running_->Off();
        resume_->Off();
        root_ = "";
        last_.store(std::time(nullptr));
    }

    OT_ASSERT(done);

    done();
    LogVerbose()(OT_PRETTY_CLASS())("Finished garbage collection.").Flush();
}

auto Root::GC::Init(
    const std::string& root,
    bool resume,
    std::uint64_t last) noexcept -> void
{
    auto lock = Lock{lock_};
    root_ = root;
    running_->Off();
    resume_->Set(resume);
    last_.store(last);
}

auto Root::GC::Run(
    const bool from,
    const Driver& to,
    SimpleCallback cb) noexcept -> bool
{
    asio_.Internal().Post(ThreadPool::General, [=, driver = &to] {
        collect_garbage(from, driver, std::move(cb));
    });

    return true;
}

auto Root::GC::Serialize(proto::StorageRoot& out) const noexcept -> void
{
    auto lock = Lock{lock_};
    out.set_lastgc(last_.load());
    out.set_gc(running_.get());
    out.set_gcroot(root_);
}

Root::GC::~GC() { Cleanup(); }
}  // namespace opentxs::storage
