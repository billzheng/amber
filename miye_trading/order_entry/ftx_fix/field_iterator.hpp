#pragma once
#include <tuple>
#include <cassert>
#include <stdint.h>
#include <tuple>

#include "../ftx_fix/field_ptr.hpp"

namespace miye::trading::fix
{

class FieldIterator
{
public:
    using FixId_t = uint32_t;
    
    constexpr static const uint8_t separator = 0x01;

public:
    constexpr FieldIterator() = default;    
    constexpr FieldIterator(const char* begin, size_t len)
    : begin_(begin),
      end_(begin + len)
    {
    }
    
    std::tuple<FixId_t, const char*, const char*> getNext() 
    {
        FixId_t fieldId {0};
        
        // sample fix field pair: 34=123456
        while (*curr_ != '=')
        {
            fieldId *= 10;
            fieldId += *curr_ - '0';
            ++curr_;
        }

        assert(curr_ < end_);
        assert (fieldId > 0); 
        
        ++curr_; // move pointer over '='
       
        const char* val_begin {curr_};
        // look for end of fix value
        while (*curr_ != separator)
        {
            ++curr_;            
        }
        
        ++curr_; // move pointer over field separator

        return std::make_tuple(fieldId, val_begin, curr_);
    }
        
    bool hasNext() const { return curr_ < end_; }

private:
    const char* begin_ {nullptr};
    const char* end_ {nullptr};
    const char* curr_ {nullptr};
};

} //namespace miye::trading::fix
