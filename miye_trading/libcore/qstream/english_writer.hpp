/*
 * english_writer.hpp
 * Purpose: output stream that writes a representation
 * of data in english
 * Author
 */

#pragma once
#include "libcore/utils/syscalls_files.hpp"
//#include "message/english.hpp"
#include "qstream_common.hpp"
#include "qstream_place.hpp"
#include "qstream_writer_interface.hpp"
#include "stdout_writer.hpp"
#include <memory>
#include <string>

namespace miye
{
namespace qstream
{

template <typename Clock>
struct english_writer : public qstream_writer_interface<english_writer<Clock>>
{
    english_writer(Clock& clock_, std::string descrip) : clk(clock_), description(descrip)
    {
        auto path = extract_path(descrip);
        if (path == "1")
        {
            fd = 1;
        }
        else if (path == "2")
        {
            fd = 2;
        }
        else
        {
            INVARIANT_FAIL("english must have path of 1 or 2");
        }
        wrapped_stdout_writer = std::make_shared<stdout_writer<Clock>>(clk, fd == 2);
    }
    place pledge(size_t required_sz) noexcept { return wrapped_stdout_writer->pledge(required_sz); }
    // override the annouce
    int announce(const place& pledged)
    {
        std::ostringstream os;
        os << time::as_utc(clk.now()) << " " << clk.now() << " " << types::message::to_english(pledged.start)
           << std::endl;
        INVARIANT_MSG(fd == 1 || fd == 2, "how did we get an fd of " << fd << "? " << std::hex << DUMP(this));
        syscalls::write(fd, os.str().c_str(), os.str().size());
        return 0; // success
    }

    std::shared_ptr<stdout_writer<Clock>> wrapped_stdout_writer;
    int fd;
    Clock& clk;
    std::string description;
};

} // namespace qstream
} // namespace miye
