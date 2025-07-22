//
// SRAVZ LLC
//

#include "parser.hpp"

parser::parser() {
    opt_.allow_comments = true;
    opt_.allow_trailing_commas = true;
}

boost::json::value parser::parse(const std::string &msg) {
    boost::json::value val = boost::json::parse(msg, {}, opt_);
    return val.as_object();
}

std::string parser::parse_http_request(http::verb &&method, std::string path,
                                       std::string body) {
    return "This is request method: " +
        boost::lexical_cast<std::string>(method) + " path: " + path +
        " body: " + body;
}