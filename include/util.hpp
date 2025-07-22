#ifndef SRAVZ_BACKENDCPP_UTIL_H
#define SRAVZ_BACKENDCPP_UTIL_H

// Includes
#include <sw/redis++/errors.h>
#include <sw/redis++/redis++.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/write.hpp>
#include <boost/atomic.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/config.hpp>
#include <boost/coroutine2/all.hpp>
#include <boost/foreach.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "boost/format.hpp"
#include "root_certificates.hpp"

// Namespaces
namespace attrs = boost::log::attributes;
namespace beast = boost::beast; // from <boost/beast.hpp>
namespace expr = boost::log::expressions;
namespace http = beast::http; // from <boost/beast/http.hpp>
namespace json = boost::json;
namespace keywords = boost::log::keywords;
namespace logging = boost::log;
namespace net = boost::asio; // from <boost/asio.hpp>
namespace po = boost::program_options;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>

/* Using namespaces */
// using boost::format;
// using boost::io::group;
using RedisInstance = sw::redis::Redis;
using tcp = boost::asio::ip::tcp;

typedef boost::asio::io_context::executor_type executor_type;
typedef boost::asio::strand<executor_type> strand;

// Classes
class ThreadInfo {
public:
    beast::flat_buffer buffer;
    std::shared_ptr<RedisInstance> redis;
    ThreadInfo() {}
    ThreadInfo(beast::flat_buffer buffer_,
               std::shared_ptr<RedisInstance> redis_)
        : buffer(buffer_), redis(redis_) {}
};
typedef std::map<boost::thread::id, ThreadInfo> ThreadsInfo;

// Functions
// Gets env variable
bool getenv(const char *name, std::string &env);
// Returns redis connection options to be used in redis connection creation
sw::redis::ConnectionOptions getRedisConnectionOptions();

#endif // end SRAVZ_BACKENDCPP_UTIL_H