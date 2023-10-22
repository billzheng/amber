/*
 * fileutils.hpp
 *
 * Purpose: file utilities, general and mmap specific
 *
 * Author: 
 */
#pragma once

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

#include "libcore/essential/platform_defs.hpp"
#include "libcore/qstream/mmap_headers.hpp"
#include "libcore/utils/stringutils.hpp"
#include "libcore/utils/syscalls_files.hpp"

namespace miye
{
namespace utils
{

inline bool file_exists(const std::string& name)
{
    struct stat buffer;
    return (::stat(name.c_str(), &buffer) == 0);
}

inline bool can_lock(const std::string& name)
{
    int fd = ::open(name.c_str(), O_RDWR);
    if (fd < 0)
    {
        return false;
    }

    auto f_ret = ::flock(fd, LOCK_EX | LOCK_NB);
    bool retval = true;
    if (f_ret < 0)
    {
        retval = false;
    }
    syscalls::close(fd);
    return retval;
}

bool iofile_truncate(const std::string& name, size_t min_size = 0)
{
    int fd = syscalls::open(name.c_str(), O_RDWR);
    auto f_ret = ::flock(fd, LOCK_EX | LOCK_NB);
    if (fd < 0 || f_ret < 0)
    {
        goto error;
    }
    else
    {
        qstream::mmap_header header;
        ssize_t n = syscalls::read(fd, &header, sizeof(header));
        if (n == sizeof(header) && header.magic == qstream::mmap_magic)
        {
            // round up the file to a cache boundary
            auto const size = std::max(min_size, header.write_offset);
            syscalls::ftruncate64(fd, ROUND_UP(size, CACHE_LINE_SIZE));

            syscalls::close(fd);
            return true;
        }
        else
        {
            std::cerr << name << " problem with magic number: "
                      << DUMP(header.magic) " != " << qstream::mmap_magic
                      << " or n [" << n << "] != sizeof(header) ["
                      << sizeof(header) << "]" << std::endl;
            syscalls::close(fd);
        }
    }
error:
    std::cerr << "Unable to open file in read state for truncate " << name;
    syscalls::close(fd);
    return false;
}

size_t find_rotate_file_index(std::string name)
{
    size_t i = 1;
    truncate_to(name, ":");
    truncate_from(name, "@");

    std::ostringstream oss;
    oss << name << "." << i;
    std::string new_name(oss.str());
    if (utils::file_exists(new_name))
    {
        do
        {
            if (!utils::file_exists(new_name))
            {
                break;
            }
            ++i;
            std::ostringstream oss;
            oss << name << "." << i;
            new_name = oss.str();
        } while (true);
        return (i - 1);
    }
    return 0;
}
inline std::string strip_io_decoration(std::string name)
{
    truncate_to(name, ":");
    truncate_from(name, "@");
    return name;
}

bool rotate_file(std::string name, bool truncate = false)
{
    truncate_to(name, ":");
    truncate_from(name, "@");

    if (utils::file_exists(name))
    {
        size_t i = 1;
        std::ostringstream oss;
        oss << name << "." << i;
        std::string new_name(oss.str());
        do
        {
            if (!utils::file_exists(new_name))
            {
                break;
            }
            ++i;
            std::ostringstream oss;
            oss << name << "." << i;
            new_name = oss.str();
        } while (true);

        if (truncate)
        {
            iofile_truncate(name);
        }

        int r = std::rename(name.c_str(), new_name.c_str());
        if (r == -1)
        {
            std::cerr << "Failed to rename " << name.c_str() << " to "
                      << new_name.c_str() << "\n";
            return false;
        }
    }
    return true;
}

inline std::string join_path(const std::string& parent,
                             const std::string& child)
{
    std::string result;
    if (parent.size() > 0)
    {
        size_t found = parent.find_last_of("/");
        if (found == std::string::npos || found < (parent.size() - 1))
        {
            result = parent;
        }
        else
        {
            result = parent.substr(0, parent.size() - 1);
        }
    }
    if (child.size() > 0)
    {
        std::string child_result = child;
        while (child_result[0] == '/' || child_result[0] == '.')
        {
            child_result = child_result.substr(1, child_result.size());
        }
        result = result + "/" + child_result;
    }
    return result;
}

inline std::string get_directory(const std::string& path)
{
    size_t found = path.find_last_of("/");
    if (found == std::string::npos)
    {
        return "";
    }
    return path.substr(0, found);
}

inline std::string get_file(const std::string& path)
{
    size_t found = path.find_last_of("/");
    if (found == std::string::npos)
    {
        return path;
    }
    ++found;
    return path.substr(found, path.size() - found);
}

inline std::string get_relative_path(const std::string& base,
                                     const std::string& test)
{
    size_t found = test.find(base);
    if (found == std::string::npos)
    {
        return test;
    }
    std::string result =
        test.substr(found + base.size(), test.size() - base.size());
    found = result.find("/");
    if (found == std::string::npos || found > 0)
    {
        return result;
    }
    return "." + result;
}

bool dir_exists(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

bool make_path(const std::string& path)
{
    if (path.empty())
    {
        return true;
    }
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);
    if (ret == 0)
    {
        return true;
    }

    switch (errno)
    {
    case ENOENT: {
        size_t pos = path.find_last_of('/');
        if (pos == std::string::npos)
        {
            return false;
        }
        if (!make_path(path.substr(0, pos)))
        {
            return false;
        }
    }
        return 0 == mkdir(path.c_str(), mode);
    case EEXIST:
        return dir_exists(path);

    default:
        return false;
    }
}

size_t get_file_size(std::string& filename)
{
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : 0;
}

} // namespace utils
} // namespace miye
