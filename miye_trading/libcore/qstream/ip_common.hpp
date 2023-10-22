/*
 * tcp_common.hpp
 * Purpose: common routines for tcp_{listener,reader,writer}
 * Author: 
 */

#pragma once
#include <stdint.h>
#include <string>

namespace miye
{
namespace qstream
{

uint16_t extract_port(std::string description);
std::string extract_ip(std::string description);
std::string extract_ifname(std::string description);
int get_ifindex(const int fd, std::string ifname);
bool is_class_D(std::string ip);
bool connect_can_fail(std::string description);
std::string ip_of_interface(std::string ifname);
void set_nonblocking(int fd);

} // namespace qstream
} // namespace miye
