/*

  Main file for the opentxs swig wrappers to define any variables
  common to all wrappers. This is included from each wrapper's OTX_.i

  This was created from the opentsx.i which once contained all wrapper
  directives. Each wrapper has been migrated to its own sub-directory.

*/

%include "inttypes.i";

// add the following to every .cxx file.
%inline %{
#ifndef SWIG_VERSION
#define SWIG_VERSION
#endif

#include "opentxs/network/zeromq/ListenCallbackSwig.hpp"
#include "opentxs/network/zeromq/PairEventCallbackSwig.hpp"
#include "opentxs/opentxs.hpp"

using Clock = opentxs::Clock;
%}

/* Parse the header file to generate wrappers */
#ifndef OPENTXS_EXPORT
#define OPENTXS_EXPORT
#endif

#ifndef OPENTXS_NO_EXPORT
#define OPENTXS_NO_EXPORT
#endif

namespace std {
  typedef uint64_t size_t;
};

namespace opentxs {
namespace network {
namespace zeromq {
    class Message;
};
};
};

typedef int64_t time64_t;

%include "../../include/opentxs/core/PasswordPrompt.hpp"
%include "../../include/opentxs/client/NymData.hpp"
%include "../../include/opentxs/Pimpl.hpp"
%include "../../include/opentxs/SharedPimpl.hpp"
%include "../../include/opentxs/Types.hpp"
%include "../../include/opentxs/core/Data.hpp"
%include "../../include/opentxs/core/Identifier.hpp"
%include "../../include/opentxs/core/identifier/Nym.hpp"
%include "../../include/opentxs/core/identifier/Server.hpp"
%include "../../include/opentxs/core/identifier/UnitDefinition.hpp"
%include "../../include/opentxs/core/crypto/OTCallback.hpp"
%include "../../include/opentxs/core/crypto/OTCaller.hpp"
%include "../../include/opentxs/ui/Widget.hpp"
%include "../../include/opentxs/ui/ListRow.hpp"
%include "../../include/opentxs/ui/BalanceItem.hpp"
%include "../../include/opentxs/ui/AccountActivity.hpp"
%include "../../include/opentxs/ui/AccountSummaryItem.hpp"
%include "../../include/opentxs/ui/IssuerItem.hpp"
%include "../../include/opentxs/ui/AccountSummary.hpp"
%include "../../include/opentxs/ui/ActivitySummaryItem.hpp"
%include "../../include/opentxs/ui/ActivitySummary.hpp"
%include "../../include/opentxs/ui/ActivityThreadItem.hpp"
%include "../../include/opentxs/ui/ActivityThread.hpp"
%include "../../include/opentxs/ui/ContactListItem.hpp"
%include "../../include/opentxs/ui/PayableListItem.hpp"
%include "../../include/opentxs/ui/ContactList.hpp"
%include "../../include/opentxs/ui/MessagableList.hpp"
%include "../../include/opentxs/ui/PayableList.hpp"
%include "../../include/opentxs/ui/ContactItem.hpp"
%include "../../include/opentxs/ui/ContactSubsection.hpp"
%include "../../include/opentxs/ui/ContactSection.hpp"
%include "../../include/opentxs/ui/Contact.hpp"
%include "../../include/opentxs/ui/ProfileItem.hpp"
%include "../../include/opentxs/ui/ProfileSubsection.hpp"
%include "../../include/opentxs/ui/ProfileSection.hpp"
%include "../../include/opentxs/ui/Profile.hpp"
%include "../../include/opentxs/network/zeromq/curve/Client.hpp"
%include "../../include/opentxs/network/zeromq/curve/Server.hpp"
%include "../../include/opentxs/network/zeromq/Frame.hpp"
%include "../../include/opentxs/network/zeromq/FrameIterator.hpp"
%include "../../include/opentxs/network/zeromq/FrameSection.hpp"
%include "../../include/opentxs/network/zeromq/Message.hpp"
%include "../../include/opentxs/network/zeromq/socket/Socket.hpp"
%include "../../include/opentxs/network/zeromq/socket/Sender.hpp"
%include "../../include/opentxs/network/zeromq/socket/Publish.hpp"
%include "../../include/opentxs/network/zeromq/ListenCallbackSwig.hpp"
%include "../../include/opentxs/network/zeromq/ListenCallback.hpp"
%include "../../include/opentxs/network/zeromq/PairEventCallbackSwig.hpp"
%include "../../include/opentxs/network/zeromq/PairEventCallback.hpp"
%include "../../include/opentxs/network/zeromq/socket/Subscribe.hpp"
%include "../../include/opentxs/network/zeromq/Proxy.hpp"
%include "../../include/opentxs/network/zeromq/socket/Pull.hpp"
%include "../../include/opentxs/network/zeromq/socket/Push.hpp"
%include "../../include/opentxs/network/zeromq/ReplyCallback.hpp"
%include "../../include/opentxs/network/zeromq/socket/Reply.hpp"
%include "../../include/opentxs/network/zeromq/socket/Request.hpp"
%include "../../include/opentxs/network/zeromq/socket/Pair.hpp"
%include "../../include/opentxs/network/zeromq/socket/Dealer.hpp"
%include "../../include/opentxs/network/zeromq/socket/Router.hpp"
%include "../../include/opentxs/network/zeromq/Context.hpp"
%include "../../include/opentxs/api/Endpoints.hpp"

// add the following to every .cxx file.
%inline %{
  using namespace opentxs;
%}
