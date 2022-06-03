// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/Basic.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <QCoreApplication>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <thread>

namespace ottest
{
using namespace std::literals::chrono_literals;

class QtApplication
{
private:
    const opentxs::Time started_;
    std::atomic<bool> running_;
    std::promise<void> promise_thread_running_;
    std::thread thread_;
    std::unique_ptr<QCoreApplication> qt_;

private:
    static void sstarter(QtApplication* app) { app->starter(); }
    void starter()
    {
        static char test[]{"test"};
        static char* argv[]{&test[0], nullptr};
        int argc{1};
        qt_.reset(new QCoreApplication(argc, argv));
        promise_thread_running_.set_value();
        if (qt_->instance()) { qt_->exec(); }
    }

public:
    auto get_core_app() { return qt_.get(); }
    auto start() noexcept -> void
    {
        if (auto running = running_.exchange(true); false == running) {
            std::future<void> future_thread_running =
                promise_thread_running_.get_future();
            thread_ = std::thread(sstarter, this);
            future_thread_running.get();
        }
    }

    auto stop() noexcept -> void
    {
        if (auto running = running_.exchange(false); running) {
            promise_thread_running_ = std::promise<void>{};
            do {
                QCoreApplication::processEvents();
            } while (QCoreApplication::hasPendingEvents());

            QCoreApplication::exit(0);

            if (thread_.joinable()) { thread_.join(); }

            qt_.release();
        }
    }

    QtApplication() noexcept
        : started_(opentxs::Clock::now())
        , running_(false)
        , promise_thread_running_()
        , thread_()
        , qt_{}
    {
    }

    ~QtApplication()
    {
        stop();
        qt_.release();
    }
};

QtApplication qt_app{};

auto GetQT() noexcept -> QObject*
{
    qt_app.start();
    return qt_app.get_core_app();
}

auto StartQT(bool) noexcept -> void { qt_app.start(); }

auto StopQT() noexcept -> void { qt_app.stop(); }
}  // namespace ottest
