// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/Legacy.hpp"  // IWYU pragma: associated
#include "api/Log.hpp"     // IWYU pragma: associated

extern "C" {
#include <android/log.h>
}

#include "opentxs/util/Allocator.hpp"

namespace opentxs::api::imp
{
auto Legacy::use_dot() noexcept -> bool { return false; }

auto Log::print(
    const int level,
    const UnallocatedCString& text,
    const UnallocatedCString& thread) noexcept -> void
{
    switch (level) {
        case 0:
        case 1: {
            __android_log_write(ANDROID_LOG_INFO, "OT Output", text.c_str());
        } break;
        case 2:
        case 3: {
            __android_log_write(ANDROID_LOG_DEBUG, "OT Debug", text.c_str());
        } break;
        case 4:
        case 5: {
            __android_log_write(
                ANDROID_LOG_VERBOSE, "OT Verbose", text.c_str());
        } break;
        default: {
            __android_log_write(
                ANDROID_LOG_UNKNOWN, "OT Unknown", text.c_str());
        } break;
    }
}
}  // namespace opentxs::api::imp

// TODO after libc++ finally incorporates this into std, and after a new version
// of the ndk is released which uses that version of libc++, then this can be
// removed
namespace std::experimental::fundamentals_v1::pmr
{
auto get_default_resource() noexcept -> opentxs::alloc::Resource*
{
    return opentxs::alloc::System();
}
}  // namespace std::experimental::fundamentals_v1::pmr
