#include <thread>

#include "exchange_base.h"

namespace dinobot { namespace exchanges {

exchange_base::exchange_base(dinobot::lib::configs &c)
    : uri_(c.get_config<std::string>("exchange_ws", "uri"))
    , path_(c.get_config<std::string>("ring_writer_md", "path"))
    , ring_mtu_(c.get_config<int>("ring_writer_md", "mtu"))
    , ring_num_readers_(c.get_config<int>("ring_writer_md", "num_readers"))
    , ring_elements_(c.get_config<int>("ring_writer_md", "elements"))
    , ring_logger_file_(c.get_config<std::string>("logs", "path_ws"))
{
}

exchange_base::~exchange_base()
{
}

/*
void exchange_base::init()
{
}

void exchange_base::start()
{
}
*/

void exchange_base::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

}} // dinobot::exchanges::binance

