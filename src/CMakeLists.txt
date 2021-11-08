# Copyright (c) 2010-2021 The Open-Transactions developers
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_library(opentxs-common OBJECT "")
target_link_libraries(opentxs-common PUBLIC opentxs::messages)

if(NOT OT_IWYU)
  target_precompile_headers(
    opentxs-common
    PUBLIC
    "$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<array$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<atomic$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cassert$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cctype$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cerrno$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<chrono$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cinttypes$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cmath$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<condition_variable$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<csignal$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdarg$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstddef$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdint$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdio$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstdlib$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<cstring$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ctime$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<deque$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<exception$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<fstream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<functional$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<future$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iomanip$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ios$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iosfwd$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iostream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<iterator$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<limits$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<list$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<locale$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<map$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<memory$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<mutex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<new$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<numeric$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<optional$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ostream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<queue$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<random$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<ratio$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<regex$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<set$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<sstream$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<stdexcept$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<string$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<thread$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<tuple$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<typeinfo$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<type_traits$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<unordered_map$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<unordered_set$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<utility$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<variant$<ANGLE-R>>"
    "$<$<COMPILE_LANGUAGE:CXX>:<vector$<ANGLE-R>>"
    [["opentxs/protobuf/TransactionData.pb.h"]]
    [["opentxs/protobuf/AcceptPendingPayment.pb.h"]]
    [["opentxs/protobuf/AccountData.pb.h"]]
    [["opentxs/protobuf/AccountEvent.pb.h"]]
    [["opentxs/protobuf/AddClaim.pb.h"]]
    [["opentxs/protobuf/AddContact.pb.h"]]
    [["opentxs/protobuf/APIArgument.pb.h"]]
    [["opentxs/protobuf/AsymmetricKey.pb.h"]]
    [["opentxs/protobuf/Authority.pb.h"]]
    [["opentxs/protobuf/Bailment.pb.h"]]
    [["opentxs/protobuf/BailmentReply.pb.h"]]
    [["opentxs/protobuf/BasketItem.pb.h"]]
    [["opentxs/protobuf/BasketParams.pb.h"]]
    [["opentxs/protobuf/Bip47Channel.pb.h"]]
    [["opentxs/protobuf/Bip47Direction.pb.h"]]
    [["opentxs/protobuf/BitcoinBlockHeaderFields.pb.h"]]
    [["opentxs/protobuf/BlindedSeriesList.pb.h"]]
    [["opentxs/protobuf/BlockchainAccountData.pb.h"]]
    [["opentxs/protobuf/BlockchainActivity.pb.h"]]
    [["opentxs/protobuf/BlockchainAddress.pb.h"]]
    [["opentxs/protobuf/BlockchainBlockHeader.pb.h"]]
    [["opentxs/protobuf/BlockchainBlockLocalData.pb.h"]]
    [["opentxs/protobuf/BlockchainDeterministicAccountData.pb.h"]]
    [["opentxs/protobuf/BlockchainEnums.pb.h"]]
    [["opentxs/protobuf/BlockchainExternalAddress.pb.h"]]
    [["opentxs/protobuf/BlockchainFilterHeader.pb.h"]]
    [["opentxs/protobuf/BlockchainHDAccountData.pb.h"]]
    [["opentxs/protobuf/BlockchainInputWitness.pb.h"]]
    [["opentxs/protobuf/BlockchainOutputMultisigDetails.pb.h"]]
    [["opentxs/protobuf/BlockchainP2PChainState.pb.h"]]
    [["opentxs/protobuf/BlockchainP2PHello.pb.h"]]
    [["opentxs/protobuf/BlockchainP2PSync.pb.h"]]
    [["opentxs/protobuf/BlockchainPeerAddress.pb.h"]]
    [["opentxs/protobuf/BlockchainPreviousOutput.pb.h"]]
    [["opentxs/protobuf/BlockchainTransactionInput.pb.h"]]
    [["opentxs/protobuf/BlockchainTransactionOutput.pb.h"]]
    [["opentxs/protobuf/BlockchainTransaction.pb.h"]]
    [["opentxs/protobuf/BlockchainTransactionProposal.pb.h"]]
    [["opentxs/protobuf/BlockchainTransactionProposedNotification.pb.h"]]
    [["opentxs/protobuf/BlockchainTransactionProposedOutput.pb.h"]]
    [["opentxs/protobuf/BlockchainWalletKey.pb.h"]]
    [["opentxs/protobuf/CashEnums.pb.h"]]
    [["opentxs/protobuf/ChildCredentialParameters.pb.h"]]
    [["opentxs/protobuf/Ciphertext.pb.h"]]
    [["opentxs/protobuf/Claim.pb.h"]]
    [["opentxs/protobuf/ClientContext.pb.h"]]
    [["opentxs/protobuf/ConnectionInfo.pb.h"]]
    [["opentxs/protobuf/ConnectionInfoReply.pb.h"]]
    [["opentxs/protobuf/ConsensusEnums.pb.h"]]
    [["opentxs/protobuf/ContactData.pb.h"]]
    [["opentxs/protobuf/ContactEnums.pb.h"]]
    [["opentxs/protobuf/ContactEvent.pb.h"]]
    [["opentxs/protobuf/ContactItem.pb.h"]]
    [["opentxs/protobuf/Contact.pb.h"]]
    [["opentxs/protobuf/ContactSection.pb.h"]]
    [["opentxs/protobuf/Context.pb.h"]]
    [["opentxs/protobuf/ContractEnums.pb.h"]]
    [["opentxs/protobuf/CreateInstrumentDefinition.pb.h"]]
    [["opentxs/protobuf/CreateNym.pb.h"]]
    [["opentxs/protobuf/Credential.pb.h"]]
    [["opentxs/protobuf/CurrencyParams.pb.h"]]
    [["opentxs/protobuf/Enums.pb.h"]]
    [["opentxs/protobuf/Envelope.pb.h"]]
    [["opentxs/protobuf/EquityParams.pb.h"]]
    [["opentxs/protobuf/EthereumBlockHeaderFields.pb.h"]]
    [["opentxs/protobuf/Faucet.pb.h"]]
    [["opentxs/protobuf/GCS.pb.h"]]
    [["opentxs/protobuf/GetWorkflow.pb.h"]]
    [["opentxs/protobuf/HDAccount.pb.h"]]
    [["opentxs/protobuf/HDPath.pb.h"]]
    [["opentxs/protobuf/HDSeed.pb.h"]]
    [["opentxs/protobuf/InstrumentRevision.pb.h"]]
    [["opentxs/protobuf/Issuer.pb.h"]]
    [["opentxs/protobuf/KeyCredential.pb.h"]]
    [["opentxs/protobuf/ListenAddress.pb.h"]]
    [["opentxs/protobuf/LucreTokenData.pb.h"]]
    [["opentxs/protobuf/MasterCredentialParameters.pb.h"]]
    [["opentxs/protobuf/ModifyAccount.pb.h"]]
    [["opentxs/protobuf/MoveFunds.pb.h"]]
    [["opentxs/protobuf/NoticeAcknowledgement.pb.h"]]
    [["opentxs/protobuf/NymIDSource.pb.h"]]
    [["opentxs/protobuf/Nym.pb.h"]]
    [["opentxs/protobuf/OTXEnums.pb.h"]]
    [["opentxs/protobuf/OTXPush.pb.h"]]
    [["opentxs/protobuf/OutBailment.pb.h"]]
    [["opentxs/protobuf/OutBailmentReply.pb.h"]]
    [["opentxs/protobuf/PairEvent.pb.h"]]
    [["opentxs/protobuf/PaymentCode.pb.h"]]
    [["opentxs/protobuf/PaymentEvent.pb.h"]]
    [["opentxs/protobuf/PaymentWorkflowEnums.pb.h"]]
    [["opentxs/protobuf/PaymentWorkflow.pb.h"]]
    [["opentxs/protobuf/PeerEnums.pb.h"]]
    [["opentxs/protobuf/PeerObject.pb.h"]]
    [["opentxs/protobuf/PeerReply.pb.h"]]
    [["opentxs/protobuf/PeerRequestHistory.pb.h"]]
    [["opentxs/protobuf/PeerRequest.pb.h"]]
    [["opentxs/protobuf/PeerRequestWorkflow.pb.h"]]
    [["opentxs/protobuf/PendingBailment.pb.h"]]
    [["opentxs/protobuf/PendingCommand.pb.h"]]
    [["opentxs/protobuf/PurseExchange.pb.h"]]
    [["opentxs/protobuf/Purse.pb.h"]]
    [["opentxs/protobuf/RPCCommand.pb.h"]]
    [["opentxs/protobuf/RPCEnums.pb.h"]]
    [["opentxs/protobuf/RPCPush.pb.h"]]
    [["opentxs/protobuf/RPCResponse.pb.h"]]
    [["opentxs/protobuf/RPCStatus.pb.h"]]
    [["opentxs/protobuf/RPCTask.pb.h"]]
    [["opentxs/protobuf/Seed.pb.h"]]
    [["opentxs/protobuf/SendMessage.pb.h"]]
    [["opentxs/protobuf/SendPayment.pb.h"]]
    [["opentxs/protobuf/ServerContext.pb.h"]]
    [["opentxs/protobuf/ServerContract.pb.h"]]
    [["opentxs/protobuf/ServerReply.pb.h"]]
    [["opentxs/protobuf/ServerRequest.pb.h"]]
    [["opentxs/protobuf/SessionData.pb.h"]]
    [["opentxs/protobuf/Signature.pb.h"]]
    [["opentxs/protobuf/SourceProof.pb.h"]]
    [["opentxs/protobuf/SpentTokenList.pb.h"]]
    [["opentxs/protobuf/StorageAccountIndex.pb.h"]]
    [["opentxs/protobuf/StorageAccounts.pb.h"]]
    [["opentxs/protobuf/StorageBip47AddressIndex.pb.h"]]
    [["opentxs/protobuf/StorageBip47ChannelList.pb.h"]]
    [["opentxs/protobuf/StorageBip47Contexts.pb.h"]]
    [["opentxs/protobuf/StorageBip47NymAddressIndex.pb.h"]]
    [["opentxs/protobuf/StorageBlockchainAccountList.pb.h"]]
    [["opentxs/protobuf/StorageBlockchainTransactions.pb.h"]]
    [["opentxs/protobuf/StorageContactAddressIndex.pb.h"]]
    [["opentxs/protobuf/StorageContactNymIndex.pb.h"]]
    [["opentxs/protobuf/StorageContacts.pb.h"]]
    [["opentxs/protobuf/StorageCredentials.pb.h"]]
    [["opentxs/protobuf/StorageEnums.pb.h"]]
    [["opentxs/protobuf/StorageIDList.pb.h"]]
    [["opentxs/protobuf/StorageIssuers.pb.h"]]
    [["opentxs/protobuf/StorageItemHash.pb.h"]]
    [["opentxs/protobuf/StorageItems.pb.h"]]
    [["opentxs/protobuf/StorageNotary.pb.h"]]
    [["opentxs/protobuf/StorageNymList.pb.h"]]
    [["opentxs/protobuf/StorageNym.pb.h"]]
    [["opentxs/protobuf/StoragePaymentWorkflows.pb.h"]]
    [["opentxs/protobuf/StoragePurse.pb.h"]]
    [["opentxs/protobuf/StorageRoot.pb.h"]]
    [["opentxs/protobuf/StorageSeeds.pb.h"]]
    [["opentxs/protobuf/StorageServers.pb.h"]]
    [["opentxs/protobuf/StorageThreadItem.pb.h"]]
    [["opentxs/protobuf/StorageThread.pb.h"]]
    [["opentxs/protobuf/StorageUnits.pb.h"]]
    [["opentxs/protobuf/StorageWorkflowIndex.pb.h"]]
    [["opentxs/protobuf/StorageWorkflowType.pb.h"]]
    [["opentxs/protobuf/StoreSecret.pb.h"]]
    [["opentxs/protobuf/SymmetricKey.pb.h"]]
    [["opentxs/protobuf/TaggedKey.pb.h"]]
    [["opentxs/protobuf/TaskComplete.pb.h"]]
    [["opentxs/protobuf/Token.pb.h"]]
    [["opentxs/protobuf/TransactionData.pb.h"]]
    [["opentxs/protobuf/UnitAccountMap.pb.h"]]
    [["opentxs/protobuf/UnitDefinition.pb.h"]]
    [["opentxs/protobuf/VerificationGroup.pb.h"]]
    [["opentxs/protobuf/VerificationIdentity.pb.h"]]
    [["opentxs/protobuf/VerificationOffer.pb.h"]]
    [["opentxs/protobuf/Verification.pb.h"]]
    [["opentxs/protobuf/VerificationSet.pb.h"]]
    [["opentxs/protobuf/VerifyClaim.pb.h"]]
    [["opentxs/protobuf/ZMQEnums.pb.h"]]
  )
endif()

if(WIN32)
  target_precompile_headers(
    opentxs-common
    PUBLIC
    <ShlObj.h>
    <WinSock2.h>
    <Windows.h>
    <direct.h>
    <xstring>
  )
endif()

if(APPLE)
  target_precompile_headers(
    opentxs-common
    PUBLIC
    [["TargetConditionals.h"]]
    <limits.h>
    <mach-o/dyld.h>
  )
endif()

add_subdirectory(api)
add_subdirectory(blind)
add_subdirectory(blockchain)
add_subdirectory(client)
add_subdirectory(contact)
add_subdirectory(core)
add_subdirectory(crypto)
add_subdirectory(display)
add_subdirectory(identity)
add_subdirectory(iterator)
add_subdirectory(license)
add_subdirectory(network)
add_subdirectory(otx)
add_subdirectory(protobuf)
add_subdirectory(rpc)
add_subdirectory(server)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(util)

include(GenerateExportHeader)
generate_export_header(
  opentxs
  EXPORT_FILE_NAME
  "${opentxs_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
  EXPORT_MACRO_NAME
  OPENTXS_EXPORT
)

set(cxx-install-headers
    "${opentxs_BINARY_DIR}/src/opentxs/opentxs_export.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/OT.hpp"
    "${opentxs_SOURCE_DIR}/include/opentxs/Types.hpp"
)
target_sources(
  opentxs-common
  PRIVATE
    "${opentxs_BINARY_DIR}/src/Version.cpp"
    "0_stdafx.hpp"
    "1_Internal.hpp"
    "2_Factory.hpp"
    "OT.cpp"
    "Proto.cpp"
    "Proto.hpp"
    "Proto.tpp"
)
target_link_libraries(opentxs PRIVATE opentxs-common)
target_sources(opentxs PRIVATE $<TARGET_OBJECTS:opentxs-common>)
set_target_properties(
  opentxs
  PROPERTIES
    VERSION
    "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}"
    SOVERSION 1
    POSITION_INDEPENDENT_CODE 1
)

if(Threads_FOUND)
  target_link_libraries(opentxs PUBLIC Threads::Threads)
endif()

if(CMAKE_DL_LIBS)
  target_link_libraries(opentxs PUBLIC ${CMAKE_DL_LIBS})
endif()

if(LIB_RT)
  target_link_libraries(opentxs PUBLIC ${LIB_RT})
endif()

if(LIB_ANDROID_LOG)
  target_link_libraries(opentxs PUBLIC ${LIB_ANDROID_LOG})
endif()

if(Boost_stacktrace_basic_FOUND)
  target_link_libraries(opentxs PUBLIC Boost::stacktrace_basic)
endif()

if(OPENTXS_STANDALONE)
  install(
    FILES ${cxx-install-headers}
    DESTINATION include/opentxs
    COMPONENT dev
  )
endif()

if(OPENTXS_STANDALONE OR OT_INSTALL_LIBRARY_DEPENDENCIES)
  install(
    TARGETS opentxs
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    EXPORT opentxs-targets
    COMPONENT main
  )
endif()

if(OT_INSTALL_LIBRARY_DEPENDENCIES)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_SUFFIX)
  get_target_property(
    Boost_filesystem_LIBRARY
    Boost::filesystem
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_iostreams_LIBRARY
    Boost::iostreams
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_system_LIBRARY
    Boost::system
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Boost_thread_LIBRARY
    Boost::thread
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    Protobuf_LITE_LIBRARY
    protobuf::libprotobuf-lite
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )
  get_target_property(
    ZLIB_LIBRARY
    ZLIB::ZLIB
    IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
  )

  if(OT_USE_VCPKG_TARGETS)
    get_target_property(
      ZMQ_LIBRARY_PATH
      "${OT_ZMQ_TARGET}"
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
  else()
    set(ZMQ_LIBRARY_PATH "${ZMQ_LIBRARY}")
  endif()

  if(OPENTXS_BUILD_TESTS)
    install(
      FILES "${GTEST_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(OPENTXS_BUILD_TESTS)

  install(
    FILES
      "${Boost_filesystem_LIBRARY}"
      "${Boost_iostreams_LIBRARY}"
      "${Boost_system_LIBRARY}"
      "${Boost_thread_LIBRARY}"
      "${Protobuf_LITE_LIBRARY}"
      "${SODIUM_LIBRARY}"
      "${ZLIB_LIBRARY}"
      "${ZMQ_LIBRARY_PATH}"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT main
  )

  if(Boost_stacktrace_basic_FOUND)
    get_target_property(
      Boost_stacktrace_basic_LIBRARY
      Boost::stacktrace_basic
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )
    install(
      FILES "${Boost_stacktrace_basic_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(DHT_EXPORT)
    message(WARNING "Installing OpenDHT dependencies not supported yet")
  endif(DHT_EXPORT)

  if(SQLITE_EXPORT)
    get_target_property(
      SQLITE3_LIBRARY
      SQLite::SQLite3
      IMPORTED_LOCATION_${BUILD_TYPE_SUFFIX}
    )

    install(
      FILES "${SQLITE3_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif(SQLITE_EXPORT)

  if(LMDB_EXPORT)
    install(
      FILES "${LMDB_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(OPENSSL_EXPORT)
    install(
      FILES "${OPENSSL_CRYPTO_LIBRARY}" "${OPENSSL_SSL_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()

  if(LIBSECP256K1_EXPORT AND NOT OT_BUNDLED_SECP256K1)
    install(
      FILES "${SECP256K1_LIBRARY}"
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
      COMPONENT main
    )
  endif()
endif()
