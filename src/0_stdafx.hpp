// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#endif

#if defined(BOOST_ALL_NO_LIB)
// NOTE some versions of clang in some build configurations will emit a "macro
// is not used" warning regarding BOOST_ALL_NO_LIB if a target is linked against
// Boost::headers but some of the translation units in the target do not
// actually include any boost headers
#endif
