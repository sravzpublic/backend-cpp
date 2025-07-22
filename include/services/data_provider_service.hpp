#pragma once
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/log/trivial.hpp>
#include <cstdlib>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "root_certificates.hpp"
#include "utils.hpp"

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp; // from <boost/asio/ip/tcp.hpp>

namespace sravz::services::data_provider {
class IDataProvider : public std::enable_shared_from_this<IDataProvider> {
public:
    IDataProvider(boost::asio::io_context &io_context)
        : io_context_(io_context),
          strand_(boost::asio::make_strand(io_context_)),
          ctx_(ssl::context{ssl::context::tlsv12_client}) {
        // This holds the root certificate used for verification
        load_root_certificates(ctx_);

        // Verify the remote server's certificate
        ctx_.set_verify_mode(ssl::verify_peer);

        sravz::utils::Getenv("EODHISTORICALDATA_API_KEY2",
                             EODHISTORICALDATA_API_KEY2);
        sravz::utils::Getenv("EODHISTORICALDATA_API_KEY",
                             EODHISTORICALDATA_API_KEY);
    }
    virtual void
    httpGet(const std::string &host, std::string const &port,
            const std::string &target,
            const std::function<void(const std::string &)> &callback) = 0;
    virtual void httpGet(const std::string &host, std::string const &port,
                         const std::string &target,
                         std::promise<std::string> &promise) = 0;
    virtual void httpGet(const std::string &target,
                         std::promise<std::string> &promise) = 0;
    virtual ~IDataProvider(){};
    const std::string PORT = "443";
    const std::string HOSTNAME = "eodhd.com";
    std::string EODHISTORICALDATA_API_KEY2;
    std::string EODHISTORICALDATA_API_KEY;

protected:
    boost::asio::io_context &io_context_;
    boost::asio::strand<boost::asio::io_context::executor_type> strand_;
    ssl::context ctx_;
    const int version_ = 11;

    void httpGet_(const std::string &host, std::string const &port,
                  const std::string &target,
                  const std::function<void(const std::string &)> &callback,
                  net::yield_context yield){};
};

class DataProvider : public IDataProvider {
public:
    DataProvider(boost::asio::io_context &io_context);
    void httpGet(const std::string &host, std::string const &port,
                 const std::string &target,
                 const std::function<void(const std::string &)> &callback);
    void httpGet(const std::string &host, std::string const &port,
                 const std::string &target, std::promise<std::string> &promise);
    void httpGet(const std::string &target, std::promise<std::string> &promise);
    ~DataProvider();

private:
    void httpGet_(const std::string &host, std::string const &port,
                  const std::string &target,
                  const std::function<void(const std::string &)> &callback,
                  net::yield_context yield);
};

} // namespace sravz::services::data_provider
