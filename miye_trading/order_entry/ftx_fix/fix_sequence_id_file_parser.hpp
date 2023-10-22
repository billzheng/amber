#pragma once

#include "libcore/utils/stringutils.hpp"
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace miye::trading::fix
{
inline std::pair<int32_t, int32_t> get_seq_num_id(const std::string& file)
{
    std::pair<int32_t, int32_t> v{};

    std::ifstream fs(file);
    std::string line;

    if (std::getline(fs, line))
    {
        if (!line.empty())
        {
            auto fields = utils::split(line, ":");
            assert(fields.size() == 2);

            size_t local_seq_num = std::stol(fields[0]);
            if (local_seq_num == 0)
            {
                local_seq_num = 1;
            }
            else
            {
                local_seq_num -= 1;
            }

            size_t remote_seq_num = std::stol(fields[1]);
            if (remote_seq_num == 0)
            {
                remote_seq_num = 1;
            }

            return std::make_pair(local_seq_num, remote_seq_num);
        }
        else
        {
            // maybe first time load the sequence file, set to weekly start value
            return std::make_pair(0, 1);
        }
    }

    return v;
}

inline void save_seq_num_id(const std::string& file, uint32_t local_seq_num, uint32_t remote_seq_num)
{
    std::ofstream fs(file);

    if (remote_seq_num < 1)
    {
        remote_seq_num = 1;
    }
    std::string line = std::to_string(local_seq_num + 1) + ":" + std::to_string(remote_seq_num);

    std::cout << "write seq to file: " << file << ", content: " << line << std::endl;
    fs << line;
}

} // namespace miye::trading::fix
