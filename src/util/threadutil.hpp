// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/core/demangle.hpp>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{

struct dstring {
    explicit dstring(const char* data)
        : str_{data}
    {
    }
    std::string str_;
};
template <typename T>

dstring dname(T*)
{
    std::string s = typeid(T).name();
    return dstring(s.data());
}

#ifdef __linux
using handle_type = std::uint64_t;
struct ThreadHandle {
    ThreadHandle();
    ThreadHandle(
        std::string&& name,
        std::string&& description,
        handle_type handle);
    std::string name_;
    std::string description_;
    handle_type native_;
};

class ThreadMonitor
{
public:
    static ThreadHandle add_current_thread(
        std::string&& name,
        std::string&& description);
    static std::string get_name();
    static std::string get_name(const ThreadHandle& handle);
    static std::string get_name(handle_type native);
    // Arbitrary maximum thread name length...
    static constexpr std::size_t max_name_length();
    static std::string default_name(std::string&& Class);
    enum class State { Unknown, Idle, Waiting, Blocked, Deadlocked };
    struct ThreadInfo {
        ThreadInfo(
            handle_type = {},
            State state = State{},
            std::string blocking_resource_str = {},
            std::string blocking_comment_str = {});
        handle_type native_;
        State state_;
        std::string blocking_resource_str_;
        std::string blocking_comment_str_;
    };
    static std::vector<ThreadInfo> get_snapshot();

private:
    static ThreadMonitor* instance();
    ~ThreadMonitor();
    ThreadHandle p_add_current_thread(
        std::string&& name,
        std::string&& description);
    ThreadHandle p_get();
    std::string p_get_name();
    std::string p_get_name(const ThreadHandle& handle);
    std::string p_get_name(handle_type native);
    std::vector<ThreadInfo> p_get_snapshot();
    ThreadInfo get_info(const ThreadHandle& handle);

private:
    ThreadMonitor();

private:
    static std::mutex mutex_;
    static ThreadMonitor* instance_;
    std::map<handle_type, ThreadHandle> handles_;
    std::set<std::string> names_;
    std::map<handle_type, ThreadInfo> thread_info_;
};

struct Mytime {
    Mytime();
    inline static auto base =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count();
    std::string str();
};

inline std::string mytime()
{
    Mytime m{};
    return m.str();
}

class ThreadDisplay
{
public:
    ThreadDisplay();
    void set_host_thread(const ThreadHandle& value);
    static void show(const ThreadMonitor::ThreadInfo& thr, std::ostream& os);
    static void show_all(std::ostream& os);

protected:
    void tdiag(std::string s1, std::string s2 = "") const noexcept;
    void tdiag(dstring&& mangled, std::string s2 = "") const noexcept;
    std::string get_class() const noexcept;

    virtual ~ThreadDisplay();

private:
    static std::mutex diagmtx;
    std::optional<ThreadHandle> thread_handle_;
};

#else

using handle_type std::uint64_t;
struct ThreadHandle {
    ThreadHandle() {}
    ThreadHandle(std::string&&, std::string&&, handle_type) {}
};

class ThreadMonitor
{
public:
    static ThreadHandle add_current_thread(std::string&&, std::string&&)
    {
        return {};
    }
    static std::string get_name() { return {}; }
    static std::string get_name(const ThreadHandle&) { return {}; }
    static std::string default_name() { return {}; }
    // Arbitrary maximum thread name length...
    static constexpr std::size_t max_name_length() noexcept { return 0; }
    static std::string default_name(std::string&& Class) noexcept { return {}; }
    enum class State { Unknown, Idle, Waiting, Blocked, Deadlocked };
    struct ThreadInfo {
        State state_;
        std::string blocking_resource_str_;
        std::string blocking_comment_str_;
    };
    static std::vector<ThreadInfo> get_snapshot() { return {}; }
};

class ThreadDisplay
{
public:
    ThreadDisplay() = default;
    virtual ~ThreadDisplay(){};
    void set_host_thread(const ThreadHandle&) {}
    void show(const ThreadInfo& thr, std::ostream& os) {}
    void show_all(std::ostream& os) {}

protected:
    void tdiag(std::string, std::string = "") const noexcept {}
    void tdiag(dstring&&, std::string = "") const noexcept {}
    std::string get_class() const noexcept { return {}; }
};

#endif

}  // namespace opentxs
