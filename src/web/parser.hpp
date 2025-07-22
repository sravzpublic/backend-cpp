//
// SRAVZ LLC
//

#ifndef SRAVZ_WEB_PARSER_HPP
#define SRAVZ_WEB_PARSER_HPP

#include <util.hpp>

class parser : public boost::enable_shared_from_this<parser> {
    boost::json::parse_options opt_{};

public:
    parser();
    boost::json::value parse(const std::string &s);
    std::string parse_http_request(http::verb &&method, std::string path,
                                   std::string body);
    enum class MsgType { SUBSCRIBE = 1, UNSUBSCRIBE = 2 };
};

#endif
