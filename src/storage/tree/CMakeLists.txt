# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(
  opentxs-storage-tree OBJECT
  "Accounts.cpp"
  "Accounts.hpp"
  "Bip47Channels.cpp"
  "Bip47Channels.hpp"
  "Contacts.cpp"
  "Contacts.hpp"
  "Contexts.cpp"
  "Contexts.hpp"
  "Credentials.cpp"
  "Credentials.hpp"
  "Issuers.cpp"
  "Issuers.hpp"
  "Mailbox.cpp"
  "Mailbox.hpp"
  "Node.cpp"
  "Node.hpp"
  "Notary.cpp"
  "Notary.hpp"
  "Nym.cpp"
  "Nym.hpp"
  "Nyms.cpp"
  "Nyms.hpp"
  "PaymentWorkflows.cpp"
  "PaymentWorkflows.hpp"
  "PeerReplies.cpp"
  "PeerReplies.hpp"
  "PeerRequests.cpp"
  "PeerRequests.hpp"
  "Root.cpp"
  "Root.hpp"
  "Seeds.cpp"
  "Seeds.hpp"
  "Servers.cpp"
  "Servers.hpp"
  "Thread.cpp"
  "Thread.hpp"
  "Threads.cpp"
  "Threads.hpp"
  "Tree.cpp"
  "Tree.hpp"
  "Units.cpp"
  "Units.hpp"
)
target_link_libraries(opentxs-storage-tree PRIVATE opentxs::messages)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-storage-tree>)
add_dependencies(opentxs-storage-tree generated_code)
