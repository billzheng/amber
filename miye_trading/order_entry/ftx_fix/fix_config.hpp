#pragma once
#include <iosfwd>
#include <string>

namespace miye::trading::fix
{

struct fix_config_t
{
    fix_config_t() = default;

    std::string sender_comp_id;        // tag 49
    std::string sender_sub_id;         // tag 50
    std::string target_comp_id{"FTX"}; // tag 56
    std::string password;              // tag 96
    std::string account;               // tag 1
};

inline std::ostream& operator<<(std::ostream& os, const fix_config_t& o)
{
    os << "sender_comp_id: " << o.sender_comp_id << ", sender_sub_id: " << o.sender_sub_id
       << ", password: " << o.password << ", account: " << o.account;
    return os;
}

} // namespace miye::trading::fix
