#ifndef util_tuning_hpp__
#define util_tuning_hpp__

//
// This is a collection of tuning constants
//
// SM_Xxx state machine update result
// ----------------------------------
// These apply to the return value of state_machine() in Worker subclasses
// and work() in Actor subclasses.
//
// SM_off allows SM processing to terminate.
// SM_ModuleName_fast is the repeat period in milliseconds for when the machine
// is to operate repeatedly. SM_ModuleName_slow is the experimental ;just in
// case' repeat period in milliseconds for when the machine would previously
// cease to operate.
//

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{

// State machine timing

// Switch off.
// Applies to Actor: Account::Imp, Accounts::Imp, Job, Index, SubchainStateData,
//  Requestor, BalanceOracle,
// Worker: AccountList, AccountTree, BlockchainAccountActivity,
//  BlockchainAccountStatus, BlockchainSelection, BlockchainStatistics,
//  ContactList, CustodialAccountActivity, FeeOracle, MessagableList,
//  NymList, PayableList, Peer, SeedTree,

constexpr static const int SM_off = -1;

// BlockOracle (Cache)
constexpr static const int SM_Cache_fast = 500;
constexpr static const int SM_Cache_slow = 1000;

// Process::Imp
constexpr static const int SM_Process_fast = 1;
constexpr static const int SM_Process_slow = 400;

// Rescan::Imp
constexpr static const int SM_Rescan_fast = 1;
constexpr static const int SM_Rescan_slow = 400;

// Scan::Imp
constexpr static const int SM_Scan_fast = 1;
constexpr static const int SM_Scan_slow = 400;

// ActivityThread
constexpr static const int SM_ActivityThread_fast = 100;
constexpr static const int SM_ActivityThread_slow = 500;

// BlockDownloader
constexpr static const int SM_BlockDownloader_fast = 20;
constexpr static const int SM_BlockDownloader_slow = 400;

// BlockIndexer
constexpr static const int SM_BlockIndexer_fast = 20;
constexpr static const int SM_BlockIndexer_slow = 400;

// HeaderDownloader
constexpr static const int SM_HeaderDownloader_fast = 20;
constexpr static const int SM_HeaderDownloader_slow = 400;

// FilterDownloader
constexpr static const int SM_FilterDownloader_fast = 20;
constexpr static const int SM_FilterDownloader_slow = 400;

// FeeSource
constexpr static const int SM_FeeSource_fast = 20;

// PeerManager
constexpr static const int SM_PeerManager_fast = 100;
constexpr static const int SM_PeerManager_slow = 1000;

// Wallet
constexpr static const int SM_Wallet_fast = 10;
constexpr static const int SM_Wallet_slow = 100;

// SyncServer
constexpr static const int SM_SyncServer_fast = 20;
constexpr static const int SM_SyncServer_slow = 400;

// SubchainStateData
constexpr static const int SM_SubchainStateData_slow = 60000;

}  // namespace opentxs

#endif  // util_tuning_hpp__
