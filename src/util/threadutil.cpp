#include <boost/core/demangle.hpp>
#include <pthread.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include "util/threadutil.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
#ifdef __linux

ThreadHandle::ThreadHandle()
    : name_{}
    , description_{}
    , native_{}
{
}
ThreadHandle::ThreadHandle(
    std::string&& name,
    std::string&& description,
    handle_type handle)
    : name_{name}
    , description_{description}
    , native_{handle}
{
}

// static
ThreadHandle ThreadMonitor::add_current_thread(
    std::string&& name,
    std::string&& description)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return instance()->p_add_current_thread(
        std::move(name), std::move(description));
}

ThreadMonitor::ThreadInfo::ThreadInfo(
    handle_type native,
    State state,
    std::string blocking_resource_str,
    std::string blocking_comment_str)
    : native_{native}
    , state_{state}
    , blocking_resource_str_{blocking_resource_str}
    , blocking_comment_str_{blocking_comment_str}
{
}

// static
std::string ThreadMonitor::get_name()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return instance()->p_get_name();
}

std::string ThreadMonitor::get_name(const ThreadHandle& handle)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return instance()->p_get_name(handle);
}

std::string ThreadMonitor::get_name(handle_type native)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return instance()->p_get_name(native);
}

// static
constexpr std::size_t ThreadMonitor::max_name_length()
{
    // Arbitrary limit for a displayable name
    return 30;
}

std::string ThreadMonitor::default_name(std::string&& Class)
{
    constexpr const char* opentxs_tag = "opentxs::";
    constexpr const auto tag_length = 9;
    while (true) {
        auto i = Class.find(opentxs_tag);
        if (i == std::string::npos) { break; }
        Class.erase(i, tag_length);
    }
    while (true) {
        auto p1 = Class.find_first_of("::");
        if (p1 == std::string::npos || p1 == Class.size() - 2) { break; }
        auto p2 = Class.find_first_of("::", p1 + 2);
        if (p2 == std::string::npos) { break; }
        Class.erase(0, p1 + 2);
    }
    if (Class.size() > max_name_length()) {
        if (auto i = Class.find("<"); i != std::string::npos) {
            Class.erase(max_name_length(), std::string::npos);
        } else {
            Class.erase(0, Class.size() - max_name_length());
        }
    }
    if (Class.find_first_of("::") == 0) { Class.erase(0, 2); }
    return Class;
}

// static
std::vector<ThreadMonitor::ThreadInfo> ThreadMonitor::get_snapshot()
{
    std::unique_lock<std::mutex> lock(mutex_);
    return instance()->p_get_snapshot();
}

// static
ThreadMonitor* ThreadMonitor::instance()
{
    if (!instance_) { instance_ = new ThreadMonitor; }
    return instance_;
}

ThreadMonitor::~ThreadMonitor() {}

ThreadHandle ThreadMonitor::p_add_current_thread(
    std::string&& name,
    std::string&& description)
{
    constexpr const unsigned max_pthread_name_len = 15;
    std::string thr_name = name;
    while (true) {
        auto idouble_colon = thr_name.find("::");
        if (idouble_colon == std::string::npos) break;
        thr_name.replace(idouble_colon, 2, "_");
    }
    for (auto idx = 0; idx < 1000; ++idx) {
        std::string scratch_name = thr_name;
        if (idx) { scratch_name += std::to_string(++idx); }

        if (scratch_name.size() > max_pthread_name_len) {
            scratch_name.erase(0, scratch_name.size() - max_pthread_name_len);
        }

        if (names_.find(scratch_name) == names_.end()) {
            thr_name = std::move(scratch_name);
            break;
        }
    }
    int e = pthread_setname_np(pthread_self(), thr_name.data());
    if (e) { std::cerr << "!!!!!!!!!!!!!!!!!!" << std::strerror(e) << "\n"; }
    ThreadHandle th{
        std::move(thr_name),
        std::move(description),
        static_cast<handle_type>(pthread_self())};
    names_.insert(th.name_);
    thread_info_[th.native_] =
        ThreadInfo{pthread_self(), State::Unknown, {}, {}};
    return handles_[th.native_] = std::move(th);
}

ThreadHandle ThreadMonitor::p_get()
{
    auto hit = handles_.find(static_cast<handle_type>(pthread_self()));
    return hit == handles_.end() ? ThreadHandle{std::string{}, std::string{}, 0}
                                 : hit->second;
}

std::string ThreadMonitor::p_get_name() { return p_get().name_; }

std::string ThreadMonitor::p_get_name(const ThreadHandle& handle)
{
    return handle.name_;
}

std::string ThreadMonitor::p_get_name(handle_type native)
{
    return handles_[native].name_;
}

ThreadMonitor::ThreadInfo ThreadMonitor::get_info(const ThreadHandle& handle)
{
    return thread_info_[handle.native_];
}

std::vector<ThreadMonitor::ThreadInfo> ThreadMonitor::p_get_snapshot()
{
    std::vector<ThreadInfo> result{};
    result.reserve(thread_info_.size());
    for (auto& p : thread_info_) { result.push_back(p.second); }
    return result;
}

ThreadMonitor::ThreadMonitor()
    : handles_{}
    , names_{}
    , thread_info_{}
{
}

std::mutex ThreadMonitor::mutex_ = {};
ThreadMonitor* ThreadMonitor::instance_ = nullptr;

Mytime::Mytime() {}
std::string Mytime::str()
{
    auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count() -
                base;
    return std::to_string(diff);
}

// static
std::mutex ThreadDisplay::diagmtx{};

ThreadDisplay::ThreadDisplay()
    : thread_handle_{}
{
}
void ThreadDisplay::set_host_thread(const ThreadHandle& value)
{
    std::unique_lock<std::mutex> l(diagmtx);
    thread_handle_ = value;
}

void ThreadDisplay::show(const ThreadMonitor::ThreadInfo& thr, std::ostream& os)
{
    std::unique_lock<std::mutex> l(diagmtx);
    std::cerr << "# " << std::left
              << std::setw(ThreadMonitor::max_name_length() + 2)
              << ThreadMonitor::get_name(thr.native_);
    std::cerr << std::left << std::setw(ThreadMonitor::max_name_length() + 2);
    std::cerr << "\n";
}

void ThreadDisplay::show_all(std::ostream& os)
{
    std::cerr << " THREADS\n";

    auto all = ThreadMonitor::get_snapshot();
    for (auto& t : all) { show(t, os); }
    std::cerr << "------\n";
}

void ThreadDisplay::tdiag(std::string s1, std::string s2) const noexcept
{
    std::unique_lock<std::mutex> l(diagmtx);
    std::string C = get_class();

    std::cerr << "#### " << mytime() << " ";
    std::cerr << std::left << std::setw(17) << ThreadMonitor::get_name();
    std::cerr << std::left << std::setw(17);
    if (thread_handle_.has_value()) {
        std::cerr << ThreadMonitor::get_name(thread_handle_.value());
    } else {
        std::cerr << "----------";
    }
    std::cerr << std::left << std::setw(80) << C;
    std::cerr << s1 << " ";
    std::cerr << s2 << "\n";
}

void ThreadDisplay::tdiag(dstring&& mangled, std::string s2) const noexcept
{
    std::unique_lock<std::mutex> l(diagmtx);
    std::string s = typeid(*this).name();
    std::string C = boost::core::demangle(s.data());

    std::string s1 = boost::core::demangle(mangled.str_.data());

    std::cerr << "#### " << mytime() << " ";
    std::cerr << std::left << std::setw(17) << ThreadMonitor::get_name();
    std::cerr << std::left << std::setw(17);
    if (thread_handle_.has_value()) {
        std::cerr << ThreadMonitor::get_name(thread_handle_.value());
    } else {
        std::cerr << "----------";
    }
    std::cerr << std::left << std::setw(80) << C;
    std::cerr << s1 << " ";
    std::cerr << s2 << "\n";
}

std::string ThreadDisplay::get_class() const noexcept
{
    std::string s = typeid(*this).name();
    std::string Class = boost::core::demangle(s.data());
    return Class;
}

ThreadDisplay::~ThreadDisplay(){

};

#endif

}  // namespace opentxs
