/*
 * exception.hpp
 *
 * Purpose: exception wrappers
 */

#include <utility>
#include <exception>
#include <sstream>

#pragma once

// the below is only defined in unittests of syscalls
// try to keep everything compileing with -fno-exceptions 
// unless the team agrees otherwise
#ifdef BREAK_FNO_EXCEPTIONS
namespace miye { namespace essential {

class exception : public std::exception
{
public:
    explicit exception(uint64_t error_type_ = exception::fatal)
        : std::exception()
        , error_type(error_type_)
    {}

    exception(const exception &rhs)
        : std::exception(rhs)
        , error_type(rhs.error_type)
    {
        errormsg.str(rhs.errormsg.str());
    }


    virtual ~exception() throw()
    {}

    virtual const char* what() const throw()
    {
        msg = std::string(errormsg.str());
        return msg.c_str();
    }

    enum
    {
        fatal         = 0,
        runtime       = 1,
        syscall_error = 2
    };

    // note std::move below
    template<typename T>
    exception&& operator<<(T const& v)
    {
        this->errormsg << v;
        return std::move(*this);
    }

    const uint64_t  error_type;
private:
    std::ostringstream errormsg;
    mutable std::string msg;
};

}}
#endif
