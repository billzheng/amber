
/*
 * stdout_writer.hpp
 * write to stdout...
 * or also stderr
 * Author: 
 */

#pragma once
#include "libcore/utils/syscalls_files.hpp"
#include "qstream_common.hpp"
#include "qstream_writer_interface.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock>
struct stdout_writer : public qstream_writer_interface<stdout_writer<Clock>>
{
    stdout_writer(Clock& clock_, bool is_stderr = false)
        : fd(0), bytes_used(0), clk(clock_)
    {
        if (is_stderr)
        {
            description = "stdout:2";
            fd = 2;
        }
        else
        {
            description = "stdout:1";
            fd = 1;
        }
    }
    stdout_writer(Clock& clock_, std::string descrip)
        : fd(0), bytes_used(0), clk(clock_), description(descrip)
    {
        auto path = extract_path(descrip);
        if (!path.compare("1"))
        {
            fd = 1;
        }
        else if (path.compare("2"))
        {
            fd = 2;
        }
        else
        {
            INVARIANT_FAIL("stdout must have path of 1 or 2");
        }
    }

    place pledge(size_t required_sz) noexcept
    {
        return place(writebuffer, required_sz);
    }

    int announce(const place& pledged)
    {
        size_t written = 0;
        while (written < pledged.size)
        {
            written += syscalls::write(
                fd, (char*)pledged.start + written, pledged.size - written);
        }
        return 0; // success
    }
    void reset_fd(int new_fd)
    {
        ASSERT(new_fd == 1 || new_fd == 2);
        fd = new_fd;
    }

    int fd;
    size_t bytes_used;
    char writebuffer[SUPERPAGE_SIZE];
    Clock& clk;
    std::string description;
};
} // namespace qstream
} // namespace miye
