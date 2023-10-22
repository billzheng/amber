
#include "logger.h"

namespace dinobot { namespace utils {

logger::logger(std::string fn)
    : filename_(fn)
    //, log_(fn, std::ofstream::app)
    , log_(fn, std::ofstream::binary)
{
    
}

logger::~logger()
{
}

bool logger::write_log(std::string message)
{
    std::lock_guard<std::mutex> lck (mtx_);
    log_ << message << std::endl;
    return true;
}

bool logger::write_log(unsigned char *message, uint32_t size)
{
    std::lock_guard<std::mutex> lck (mtx_);
    //log_ << size << std::endl;
    //log_ << message << std::endl;
	log_.write(reinterpret_cast<const char *>(&size), sizeof(size));
	log_.write(reinterpret_cast<const char *>(message), size);
    return true;
}

bool logger::write_log(const char *message, uint32_t size)
{
    std::lock_guard<std::mutex> lck (mtx_);
	log_.write(reinterpret_cast<const char *>(&size), sizeof(size));
	log_.write(message, size);
    //log_ << size << std::endl;
    //log_ << message << std::endl;
    return true;
}

}} // dinobot::utils 
