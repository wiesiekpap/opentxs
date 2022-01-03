// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Basic.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <QCoreApplication>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace ottest
{
class QtApplication
{
private:
    const opentxs::Time started_;
    std::atomic<bool> running_;
    std::promise<QObject*> promise_;
    std::thread thread_;

public:
    std::shared_future<QObject*> future_;

    auto start() noexcept -> void
    {
        if (auto running = running_.exchange(true); false == running) {
            thread_ = std::thread{[this] {
                char test[]{"test"};
                char* argv[]{&test[0], nullptr};
                int argc{1};
                auto qt = QCoreApplication(argc, argv);
                promise_.set_value(&qt);
                qt.exec();
            }};
        }
    }

    auto stop() noexcept -> void
    {
        if (auto running = running_.exchange(false); running) {
            future_.get();
            // FIXME find correct way to shut down QCoreApplication without
            // getting random segfaults
            static constexpr auto delay = std::chrono::seconds{4};
            static constexpr auto zero = std::chrono::microseconds{0};
            const auto elapsed = opentxs::Clock::now() - started_;
            const auto wait =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    delay - elapsed);
            opentxs::Sleep(std::max(wait, zero));
            QCoreApplication::exit(0);

            if (thread_.joinable()) { thread_.join(); }

            promise_ = {};
            future_ = promise_.get_future();
        }
    }

    QtApplication() noexcept
        : started_(opentxs::Clock::now())
        , running_(false)
        , promise_()
        , thread_()
        , future_(promise_.get_future())
    {
    }

    ~QtApplication() { stop(); }
};

QtApplication qt_{};

auto GetQT() noexcept -> QObject*
{
    qt_.start();

    return qt_.future_.get();
}

auto StartQT(bool) noexcept -> void { qt_.start(); }

auto StopQT() noexcept -> void { qt_.stop(); }
}  // namespace ottest
