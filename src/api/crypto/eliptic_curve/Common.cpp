//
// Created by PetterFi on 09.08.22.
//
#include "internal/api/crypto/eliptic_curve/Common.hpp"

namespace opentxs::api::crypto::eliptic_curve
{

std::string_view get_pubkey_prefix(
    std::string_view pubkey_sv,
    std::size_t const prefix_size)
{
    static constexpr auto not_correct_result{"O_o"};
    return pubkey_sv.size() > prefix_size ? pubkey_sv.substr(0U, prefix_size)
                                          : not_correct_result;
}

std::string add_leading_zeros(
    std::string const& value,
    std::size_t const desired_size)
{
    auto const leading_zeros_number = desired_size - value.size();
    std::string leading_zeros(leading_zeros_number, '0');
    return leading_zeros + value;
}

}  // namespace opentxs::api::crypto::eliptic_curve
