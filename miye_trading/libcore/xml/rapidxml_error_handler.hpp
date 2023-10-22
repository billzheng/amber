#pragma once

#include "rapidxml.hpp"
#include "libcore/utils/logging.hpp"

namespace rapidxml
{
    inline void parse_error_handler(const char *what, void *where)
    {
    	using namespace miye;
        if(miye::utils::log_opts.size() > 0) {
            LOG_ERROR("rapidxml parse error: " << what << " at: " << where);
        }
        else{
            std::cerr << "rapidxml parse error: " << what << " at: " << where << std::endl;
        }

    }
}
