//
// Created by PetterFi on 09.08.22.
//

#include <cstdint>
#include <string_view>

namespace opentxs::api::crypto::eliptic_curve::secp256k1
{

std::string uncompress_pubkey(std::string_view compressed_pubkey_sv);
bool is_compressed_pubkey_correct_size(std::size_t const pubkey_length);
bool is_pubkey_compressed(std::string_view key_prefix);
bool is_pubkey_starts_from_even_prefix(std::string_view key_prefix);
bool is_pubkey_starts_from_odd_prefix(std::string_view key_prefix);

}  // namespace
   // opentxs::api::crypto::eliptic_curve::secp256k1
