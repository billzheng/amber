#include <string>
#include <cstdio>
#include <sstream>
#include <iomanip>

#include "ring_logger.h"

namespace dinobot { namespace lib { namespace shm {

ring_logger::ring_logger(std::string fn, std::string log_fn, uint32_t reader_id)
    : running_(false)
    , fn_(fn)
    , log_fn_(log_fn)
    , reader_id_(reader_id)
    , log_file_(log_fn, std::ofstream::binary)
{
    log_ = new lib::shm::ring_reader(fn_, reader_id_);
}

ring_logger::~ring_logger()
{
    log_file_.close();
}

void ring_logger::stop()
{
    running_ = false; 
}

void ring_logger::start()
{
    running_ = true;

    // write the files from the rings 
    ring_reader_retval ret;
    std::ostringstream t;

    while (running_)
    {
        ret = log_->read();

        // TODO verify this is all good when time stamps are one digit smaller
        //log_file_ << std::setfill('0') << std::setw(19) << ret.writer_timestamp << " " << std::endl;
        log_file_ << ret.writer_timestamp << " ";
        

        if (ret.chunks == 0)
        {
            log_file_.write(ret.buffer, ret.size);
            //std::cout << "testi ret chunk -0 " << std::endl;
        }
        else
        {
            while (ret.chunks != 0)
            {
                ret = log_->read();
                log_file_.write(ret.buffer, ret.size);
                //std::cout << "testi ret chunk !=0 " << std::endl;
            }
        }
        log_file_ << std::endl;
    }
}

} } } /// dinobot

