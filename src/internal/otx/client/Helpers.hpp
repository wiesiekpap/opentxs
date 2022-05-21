// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iostream>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
// Reads from cin until Newline.
inline UnallocatedCString OT_CLI_ReadLine()
{
    UnallocatedCString line;
    if (std::getline(std::cin, line)) { return line; }

    return "";
}

// Reads from cin until EOF. (Or until the ~ character as the first character on
// a line.)
inline UnallocatedCString OT_CLI_ReadUntilEOF()
{
    UnallocatedCString result("");

    for (;;) {
        UnallocatedCString input_line("");
        if (std::getline(std::cin, input_line, '\n')) {
            input_line += "\n";

            if (input_line[0] == '~')  // This is our special "break" character
                                       // for multi-line input.
                break;

            result += input_line;
        } else {
            opentxs::LogError()(": getline() was unable to "
                                "read a string from std::cin.")
                .Flush();
            break;
        }
        if (std::cin.eof() || std::cin.fail() || std::cin.bad()) {
            std::cin.clear();
            break;
        }
    }

    return result;
}
}  // namespace opentxs
