# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-core-contract-basket OBJECT
  "Basket.cpp"
  "BasketContract.cpp"
  "BasketItem.cpp"
  "BasketContract.hpp"
)
set(cxx-install-headers
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/Basket.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/BasketContract.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/core/contract/basket/BasketItem.hpp"
)
target_link_libraries(opentxs-core-contract-basket PRIVATE opentxs::messages)
target_include_directories(
  opentxs-core-contract-basket SYSTEM PRIVATE "${opentxs_SOURCE_DIR}/deps/"
)
target_sources(opentxs-core-contract-basket PRIVATE ${cxx-install-headers})
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-core-contract-basket>)
add_dependencies(opentxs-core-contract-basket generated_code)

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs/core/contract/basket
    COMPONENT dev
  )
endif()
