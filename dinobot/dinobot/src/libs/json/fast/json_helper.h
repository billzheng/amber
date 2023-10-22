#ifndef _SAJSON_HELPERS_H
#define _SAJSON_HELPERS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "sajson.h"
#pragma GCC diagnostic pop

/* 
 * This file will have some boilerplate code to help us deal with the 
 * sajson shortcommings 
 */
namespace dinobot { namespace libs { namespace json {

void debug(const sajson::value& node)
{
    using namespace sajson;

    switch (node.get_type()) {
        case TYPE_NULL:
	    std::cout << "type null" << std::endl;
            break;
        case TYPE_FALSE:
	    std::cout << "type false" << std::endl;
            break;
        case TYPE_TRUE:
	    std::cout << "type true" << std::endl;
            break;
        case TYPE_ARRAY:
	    std::cout << "type array " << std::endl;
            break;
        case TYPE_OBJECT:
	    std::cout << "type object" << std::endl;
            break;
        case TYPE_STRING:
	    std::cout << "type string" << std::endl;
            break;
        case TYPE_DOUBLE:
	    std::cout << "type double " << std::endl;
            break;
        case TYPE_INTEGER:
	    std::cout << "type integer" << std::endl;
            break;
        default:
            assert(false && "unknown node type");
    }
}


/*
 * There are cases where sajson will fail if we expect a
 * float/double and if the value actually is a integer number
 * the parsing will fail, so this function is a helper to alleviate this.
 */
double get_double_value(const sajson::value& node)
{
    if (node.get_type() == sajson::TYPE_DOUBLE)
    {
	    return node.get_double_value();
    }
    return static_cast<double>(node.get_integer_value());
}

/*
 * the implementation of sajson does not deal with 64 bit numbers. 
 * Well it does, but its claim is that most other implementations can not.
 * so here is a helper funtion to fix that (sort of)
 */
int64_t get_64bit_value(const sajson::value &node)
{
    int64_t val = 0; 
    bool ret = node.get_int53_value(&val);

    if (ret)
        return val;

    // Not sure what to do here, crash hard ? for now yeah crash and throw. 
    // other option is to get sajson to deal with 64bits... 
    throw std::runtime_error("sajson_helper::get_64_bit_value:: number we got was larger than 53 bits, can't process, exiting ");
}

}}} // dinobot::libs::json

#endif // _SAJSON_HELPERS_H
