// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UTIL_SIGNALS_HPP
#define OPENTXS_UTIL_SIGNALS_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <memory>
#include <thread>

#include "opentxs/core/Flag.hpp"

namespace opentxs
{
class Flag;

class Signals
{
public:
    OPENTXS_EXPORT static void Block();

    Signals(const Flag& running);

    ~Signals();

private:
    static const std::map<int, std::function<bool()>> handler_;

    const Flag& running_;
    std::unique_ptr<std::thread> thread_{nullptr};

    /** SIGHUP */
    static bool handle_1() { return ignore(); }
    /** SIGINT */
    static bool handle_2() { return shutdown(); }
    /** SIGQUIT */
    static bool handle_3() { return shutdown(); }
    /** SIGILL */
    static bool handle_4() { return shutdown(); }
    /** SIGTRAP */
    static bool handle_5() { return ignore(); }
    /** SIGABRT, SIGIOT */
    static bool handle_6() { return ignore(); }
    /** SIGBUS */
    static bool handle_7() { return ignore(); }
    /** SIGFPE */
    static bool handle_8() { return ignore(); }
    /** SIGKILL */
    static bool handle_9() { return shutdown(); }
    /** SIGUSR1 */
    static bool handle_10() { return ignore(); }
    /** SIGSEGV */
    static bool handle_11() { return ignore(); }
    /** SIGUSR2 */
    static bool handle_12() { return ignore(); }
    /** SIGPIPE */
    static bool handle_13() { return ignore(); }
    /** SIGALRM */
    static bool handle_14() { return ignore(); }
    /** SIGTERM */
    static bool handle_15() { return shutdown(); }
    /** SIGSTKFLT */
    static bool handle_16() { return ignore(); }
    /** SIGCLD, SIGCHLD */
    static bool handle_17() { return ignore(); }
    /** SIGCONT */
    static bool handle_18() { return ignore(); }
    /** SIGSTOP */
    static bool handle_19() { return shutdown(); }
    /** SIGTSTP */
    static bool handle_20() { return ignore(); }
    /** SIGTTIN */
    static bool handle_21() { return ignore(); }
    /** SIGTTOU */
    static bool handle_22() { return ignore(); }
    /** SIGURG */
    static bool handle_23() { return ignore(); }
    /** SIGXCPU */
    static bool handle_24() { return ignore(); }
    /** SIGXFSZ */
    static bool handle_25() { return ignore(); }
    /** SIGVTALRM */
    static bool handle_26() { return ignore(); }
    /** SIGPROF */
    static bool handle_27() { return ignore(); }
    /** SIGWINCH */
    static bool handle_28() { return ignore(); }
    /** SIGPOLL, SIGIO */
    static bool handle_29() { return ignore(); }
    /** SIGPWR */
    static bool handle_30() { return ignore(); }
    /** SIGSYS, SIGUNUSED */
    static bool handle_31() { return shutdown(); }
    static bool ignore() { return false; }
    static bool shutdown();

    void handle();
    bool process(const int signal);

    Signals() = delete;
    Signals(const Signals&) = delete;
    Signals(Signals&&) = delete;
    Signals& operator=(const Signals&) = delete;
    Signals& operator=(Signals&&) = delete;
};
}  // namespace opentxs

#endif
