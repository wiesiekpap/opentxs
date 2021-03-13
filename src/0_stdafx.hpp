// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ShlObj.h>
#include <WinSock2.h>
#include <Windows.h>
#include <direct.h>
#include <xstring>
#endif

#ifdef TARGET_OS_MAC
#include <limits.h>
#include <mach-o/dyld.h>
#endif

#include <cstdint>
#include <memory>

#ifdef __cplusplus
extern "C" {
#endif

using BYTE = std::uint8_t;
using USHORT = uint16_t;

#ifdef __cplusplus
}
#endif

// NOTE: Turns out moneypunct kind of sucks.
// As a result, for internationalization purposes,
// these values have to be set here before compilation.
//
#define OT_THOUSANDS_SEP ","
#define OT_DECIMAL_POINT "."

#ifdef _WIN32
#ifndef NO_OT_PCH
#else
#undef NO_OT_PCH
#endif
#endif

#ifndef CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS
#define CHAISCRIPT_NO_THREADS_WARNING
#endif

#ifndef OT_USE_CHAI_STDLIB
#define OT_USE_CHAI_STDLIB
#endif

#ifndef OPENTXS_PASSWORD_LEN
#define OPENTXS_PASSWORD_LEN 128
#endif

#if defined(unix) || defined(__unix__) || defined(__unix) ||                   \
    defined(__APPLE__) || defined(linux) || defined(__linux) ||                \
    defined(__linux__)
#define PREDEF_PLATFORM_UNIX 1
#endif

#if defined(debug) || defined(_DEBUG) || defined(DEBUG)
#define PREDEF_MODE_DEBUG 1
#endif
