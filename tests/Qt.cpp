// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Basic.hpp"  // IWYU pragma: associated

#include <QCoreApplication>
#include <future>
#include <thread>

namespace ottest
{
std::promise<QObject*> promise_;
std::shared_future<QObject*> future_{promise_.get_future()};

auto GetQT() noexcept -> QObject* { return future_.get(); }

auto StartQT(bool lowlevel) noexcept -> std::thread
{
    return std::thread{[=] {
        char test[]{"test"};
        char* argv[]{&test[0], nullptr};
        int argc{1};
        auto qt = QCoreApplication(argc, argv);
        promise_.set_value(&qt);
        qt.exec();
    }};
}

auto StopQT() noexcept -> void { QCoreApplication::exit(0); }
}  // namespace ottest
