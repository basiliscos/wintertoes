#ifndef LOGGING_H
#define LOGGING_H

#include <boost/log/sources/logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/expressions.hpp>


namespace src = boost::log::sources;
namespace logging = boost::log;
namespace keywords = boost::log::keywords;

enum severity_level {
    debug,
    info,
    warning,
    error,
};

namespace Apollo {
    namespace Logging {



#define LOG(s, message) { \
    src::severity_logger< severity_level > slg; \
    logging::record rec = slg.open_record(keywords::severity = s); \
    if (rec) \
    { \
        logging::record_ostream strm(rec); \
        strm << message; \
        strm.flush(); \
        slg.push_record(boost::move(rec)); \
    } \
}\

#define LOG_DEBUG(m) LOG(severity_level::debug, m)
#define LOG_INFO(m) LOG(severity_level::info, m)
#define LOG_WARN(m) LOG(severity_level::warning, m)
#define LOG_ERROR(m) LOG(severity_level::error, m)




        void init();
    }
}

#endif
