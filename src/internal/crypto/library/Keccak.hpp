//
// Created by PetterFi on 28.07.22.
//

#pragma once

#include "opentxs/crypto/library/HashingProvider.hpp"

namespace opentxs::crypto
{
class Keccak : virtual public HashingProvider
{
public:
    ~Keccak() override = default;

protected:
    Keccak() = default;

private:
    Keccak(const Keccak&) = delete;
    Keccak(Keccak&&) = delete;
    auto operator=(const Keccak&) -> Keccak& = delete;
    auto operator=(Keccak&&) -> Keccak& = delete;
};
}  // namespace opentxs::crypto
