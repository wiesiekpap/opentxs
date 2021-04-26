// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SECRET_HPP
#define OPENTXS_CORE_SECRET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"

namespace opentxs
{
class Secret;

using OTSecret = Pimpl<Secret>;
}  // namespace opentxs

namespace opentxs
{
OPENTXS_EXPORT bool operator==(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator==(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT bool operator<(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT bool operator>(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator>(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(const OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(
    const OTSecret& lhs,
    const ReadView& rhs) noexcept;
OPENTXS_EXPORT Secret& operator+=(OTSecret& lhs, const Secret& rhs) noexcept;
OPENTXS_EXPORT Secret& operator+=(OTSecret& lhs, const ReadView rhs) noexcept;

class OPENTXS_EXPORT Secret
{
public:
    enum class Mode : bool { Mem = true, Text = false };

    virtual bool operator==(const Secret& rhs) const noexcept = 0;
    virtual bool operator==(const ReadView rhs) const noexcept = 0;
    virtual bool operator!=(const Secret& rhs) const noexcept = 0;
    virtual bool operator!=(const ReadView rhs) const noexcept = 0;
    virtual bool operator<(const Secret& rhs) const noexcept = 0;
    virtual bool operator<(const ReadView rhs) const noexcept = 0;
    virtual bool operator>(const Secret& rhs) const noexcept = 0;
    virtual bool operator>(const ReadView rhs) const noexcept = 0;
    virtual bool operator<=(const Secret& rhs) const noexcept = 0;
    virtual bool operator<=(const ReadView& rhs) const noexcept = 0;
    virtual bool operator>=(const Secret& rhs) const noexcept = 0;
    virtual bool operator>=(const ReadView& rhs) const noexcept = 0;
    virtual ReadView Bytes() const noexcept = 0;
    virtual const std::byte* data() const noexcept = 0;
    virtual bool empty() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;

    virtual Secret& operator+=(const Secret& rhs) noexcept = 0;
    virtual Secret& operator+=(const ReadView rhs) noexcept = 0;

    virtual void Assign(const Secret& source) noexcept = 0;
    virtual void Assign(const ReadView source) noexcept = 0;
    virtual void Assign(const void* data, const std::size_t& size) noexcept = 0;
    virtual void AssignText(const ReadView source) noexcept = 0;
    virtual void clear() noexcept = 0;
    virtual void Concatenate(const Secret& source) noexcept = 0;
    virtual void Concatenate(const ReadView data) noexcept = 0;
    virtual void Concatenate(
        const void* data,
        const std::size_t size) noexcept = 0;
    virtual std::byte* data() noexcept = 0;
    virtual std::size_t Randomize(const std::size_t bytes) noexcept = 0;
    virtual std::size_t Resize(const std::size_t size) noexcept = 0;
    virtual AllocateOutput WriteInto(
        const std::optional<Mode> = {}) noexcept = 0;

    virtual ~Secret() = default;

protected:
    Secret() = default;

private:
    friend OTSecret;

#ifdef _WIN32
public:
#endif
    virtual Secret* clone() const noexcept = 0;
#ifdef _WIN32
private:
#endif

    Secret(const Secret& rhs) = delete;
    Secret(Secret&& rhs) = delete;
    Secret& operator=(const Secret& rhs) = delete;
    Secret& operator=(Secret&& rhs) = delete;
};
}  // namespace opentxs

#ifndef SWIG
namespace std
{
template <>
struct OPENTXS_EXPORT less<opentxs::OTSecret> {
    bool operator()(const opentxs::OTSecret& lhs, const opentxs::OTSecret& rhs)
        const;
};
}  // namespace std
#endif
#endif
