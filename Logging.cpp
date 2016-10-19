#include "Logging.h"

#include <boost/make_shared.hpp>

#include <boost/core/null_deleter.hpp>
#include <boost/log/expressions/keyword_fwd.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>


namespace keywords = boost::log::keywords;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;

//using namespace Apollo::Logging;

// The operator is used for regular stream formatting
std::ostream& operator<< (std::ostream& strm, severity_level level)
{
    static const char* strings[] =
    {
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
    };

    if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        strm << strings[level];
    else
        strm << static_cast< int >(level);

    return strm;
}



void Apollo::Logging::init(){
    boost::shared_ptr< sinks::text_ostream_backend > backend =
            boost::make_shared< sinks::text_ostream_backend >();
    backend->add_stream(
            boost::shared_ptr< std::ostream >(&std::clog, boost::null_deleter()));

    typedef sinks::synchronous_sink< sinks::text_ostream_backend > sink_t;
    boost::shared_ptr< sink_t > sink_text(new sink_t(backend));
    sink_text->set_formatter
    (
        expr::format("%1% [%2% :: %3%] - %4%")
        % expr::attr< unsigned int >("LineID")
        % expr::attr< severity_level >("Severity")
        % expr::attr< boost::posix_time::ptime >("TimeStamp")
        % expr::smessage
    );

    logging::core::get()->add_sink(sink_text);
    logging::add_common_attributes();
}
