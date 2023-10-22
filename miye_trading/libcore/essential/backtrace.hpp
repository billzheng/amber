/*
 * backtrace.hpp
 *
 * Purpose: program stack backtrace
 */


#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <cxxabi.h>
#include <execinfo.h>


#pragma once

//#LINKFLAGS=-rdynamic

namespace miye { namespace backtrace {

template<size_t DEPTH = 30>
std::vector<std::string> backtrace()
{
    std::vector<std::string>    trace;
    void                        *btrace[DEPTH];
    size_t                      n;
    char                        **s;

    std::string GREEN = "";
    std::string RED = "";
    std::string BLUE = "";
    std::string RESET = "";

    if(::isatty(::fileno(stderr)))
    {
        GREEN = "\033[38;5;30m";
        RED = "\033[38;5;196m";
        BLUE = "\033[38;5;4m";
        RESET = "\033[0m";
    }

    n = ::backtrace(btrace, DEPTH);
    s = ::backtrace_symbols(btrace, n);
    // allocation failure should never happen.
    if (!s) {
        ::abort();
    }
    // skip this function in the trace, start at 1
    for (size_t i = 1; i < n; i++) {
        char *sym = s[i];
        char *start_bracket = ::strchrnul(sym, '(');
        char *plus_pos = ::strchrnul(sym, '+');
        char *end_bracket = ::strchrnul(sym, ')');
        if (start_bracket and end_bracket and plus_pos) {
            if(plus_pos > start_bracket + 1) {
                std::string func_name(start_bracket + 1, plus_pos);
                int status = 0;
                char *demangled = abi::__cxa_demangle(func_name.c_str(), 0, 0, &status);
                if(demangled) {
                    // highlight args, function name and class in separate colours
                    std::string dmang(demangled);
                    size_t end_args_pos = dmang.find_last_of(')');
                    int bracket_count = 1;
                    size_t func_end_pos = end_args_pos - 1;
                    while(bracket_count) {
                        switch(dmang[func_end_pos])
                        {
                        case ')':
                            ++bracket_count;
                            break;
                        case '(':
                            --bracket_count;
                            break;
                        default:
                            break;
                        }
                        if(bracket_count == 0)
                        {
                            break;
                        }
                        else
                        {
                            --func_end_pos;
                        }
                    }
                    size_t func_start_pos = dmang.find_last_of(':', func_end_pos) + 1;
                    std::stringstream ss;

                    ss << "[" << i - 1 << "] " << GREEN << dmang.substr(0, func_start_pos)
                            << RED << dmang.substr(func_start_pos, func_end_pos - func_start_pos)
                            << BLUE << dmang.substr(func_end_pos, dmang.size())
                            << RESET << " ";
                    trace.push_back(ss.str());
                    continue;
                }
            }
        }
        std::stringstream ss;
        ss << "[" << i - 1 << "] " << sym;
        trace.push_back(ss.str());
    }
    ::free(s);
    return trace;
}

}}

