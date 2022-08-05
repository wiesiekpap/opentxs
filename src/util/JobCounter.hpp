// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
class JobCounter;

class Outstanding
{
    struct Imp;

public:
    auto is_limited() const noexcept -> bool;

    auto operator++() noexcept -> Outstanding&;
    auto operator--() noexcept -> Outstanding&;
    auto wait_for_finished() noexcept -> void;

    Outstanding(Imp* imp) noexcept;
    Outstanding(Outstanding&& rhs) noexcept;

    ~Outstanding();

private:
    friend JobCounter;

    Imp* imp_;

    Outstanding(const Outstanding&) = delete;
    auto operator=(const Outstanding&) -> Outstanding& = delete;
    auto operator=(Outstanding&&) -> Outstanding& = delete;
};

class JobCounter
{
public:
    auto Allocate(int limit = 0) noexcept -> Outstanding;

    JobCounter() noexcept;

    ~JobCounter();

private:
    friend Outstanding;

    struct Imp;

    Imp* imp_;

    JobCounter(const JobCounter&) = delete;
    JobCounter(JobCounter&&) = delete;
    auto operator=(const JobCounter&) -> JobCounter& = delete;
    auto operator=(JobCounter&&) -> JobCounter& = delete;
};
}  // namespace opentxs
