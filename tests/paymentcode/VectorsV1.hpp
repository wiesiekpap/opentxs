// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <vector>

namespace
{
struct VectorV1 {
    std::string words_{};
    std::string payment_code_{};
    std::vector<std::string> receiving_address_{};
    std::string private_key_{};
    std::string outpoint_{};
    std::string blinded_{};
};

struct VectorsV1 {
    VectorV1 alice_{};
    VectorV1 bob_{};
};

static const VectorsV1 vectors_1_{
    {"response seminar brave tip suit recall often sound stick owner lottery "
     "motion",
     "PM8TJTLJbPRGxSbc8EJi42Wrr6QbNSaSSVJ5Y3E4pbCYiTHUskHg13935Ubb7q8tx9GVbh2Uu"
     "RnBc3WSyJHhUrw8KhprKnn9eDznYGieTzFcwQRya4GA",
     {},
     "1b7a10f45118e2519a8dd46ef81591c1ae501d082b6610fdda3de7a3c932880d",
     "86f411ab1c8e70ae8a0795ab7a6757aea6e4d5ae1826fc7b8f00c597d500609c01000000",
     "010002063e4eb95e62791b06c50e1a3a942e1ecaaa9afbbeb324d16ae6821e091611fa96c"
     "0cf048f607fe51a0327f5e2528979311c78cb2de0d682c61e1180fc3d543b000000000000"
     "00000000000000"},
    {"reward upper indicate eight swift arch injury crystal super wrestle "
     "already dentist",
     "PM8TJS2JxQ5ztXUpBBRnpTbcUXbUHy2T1abfrb3KkAAtMEGNbey4oumH7Hc578WgQJhPjBxte"
     "Q5GHHToTYHE3A1w6p7tU6KSoFmWBVbFGjKPisZDbP97",
     {
         "141fi7TY3h936vRUKh1qfUZr8rSBuYbVBK",
         "12u3Uued2fuko2nY4SoSFGCoGLCBUGPkk6",
         "1FsBVhT5dQutGwaPePTYMe5qvYqqjxyftc",
         "1CZAmrbKL6fJ7wUxb99aETwXhcGeG3CpeA",
         "1KQvRShk6NqPfpr4Ehd53XUhpemBXtJPTL",
         "1KsLV2F47JAe6f8RtwzfqhjVa8mZEnTM7t",
         "1DdK9TknVwvBrJe7urqFmaxEtGF2TMWxzD",
         "16DpovNuhQJH7JUSZQFLBQgQYS4QB9Wy8e",
         "17qK2RPGZMDcci2BLQ6Ry2PDGJErrNojT5",
         "1GxfdfP286uE24qLZ9YRP3EWk2urqXgC4s",
     },
     "",
     "",
     ""},
};
}  // namespace
