// Sravz LLC
#include "data_provider_service.hpp"

namespace sravz::services::data_provider {

DataProvider::DataProvider(boost::asio::io_context &io_context)
    : IDataProvider(io_context) {}

void DataProvider::httpGet_(
    const std::string &host, std::string const &port, const std::string &target,
    const std::function<void(const std::string &)> &callback,
    net::yield_context yield) {
    beast::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver(io_context_);
    beast::ssl_stream<beast::tcp_stream> stream(io_context_, ctx_);

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
        ec.assign(static_cast<int>(::ERR_get_error()),
                  net::error::get_ssl_category());
        BOOST_LOG_TRIVIAL(error) << ec.message();
        return;
    }

    // Look up the domain name
    auto const results = resolver.async_resolve(host, port, yield[ec]);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "resolve"
                                 << ": " << ec.message();
        return;
    }

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    get_lowest_layer(stream).async_connect(results, yield[ec]);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "connect"
                                 << ": " << ec.message();
        return;
    }

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    stream.async_handshake(ssl::stream_base::client, yield[ec]);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "handshake"
                                 << ": " << ec.message();
        return;
    }

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version_};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream, req, yield[ec]);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "write"
                                 << ": " << ec.message();
        return;
    }

    // This buffer is used for reading and must be persisted
    beast::flat_buffer b;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::async_read(stream, b, res, yield[ec]);
    if (ec) {
        BOOST_LOG_TRIVIAL(error) << "read"
                                 << ": " << ec.message();
        return;
    }
    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Gracefully close the stream
    stream.async_shutdown(yield[ec]);
    if (ec == net::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec = {};
    }
    // if (ec) {
    //     BOOST_LOG_TRIVIAL(error) << "shutdown"
    //                              << ": " << ec.message();
    //     return;
    // }

    callback(boost::beast::buffers_to_string(res.body().data()));
}

void DataProvider::httpGet(
    const std::string &host, std::string const &port, const std::string &target,
    const std::function<void(const std::string &)> &callback) {
    auto spawn_op = boost::asio::spawn(
        strand_,
        [host, port, target, callback, this](boost::asio::yield_context yield) {
            httpGet_(host, port, target, callback, yield);
        });
    (void)spawn_op;
}

void DataProvider::httpGet(const std::string &host, std::string const &port,
                           const std::string &target,
                           std::promise<std::string> &promise) {
    auto spawn_op = boost::asio::spawn(
        strand_,
        [host, port, target, &promise, this](boost::asio::yield_context yield) {
            httpGet_(
                host, port, target,
                [&promise](const std::string &response_) {
                    promise.set_value(response_);
                },
                yield);
        });
    (void)spawn_op;
}

void DataProvider::httpGet(const std::string &target,
                           std::promise<std::string> &promise) {
    auto spawn_op = boost::asio::spawn(
        strand_, [target, &promise, this](boost::asio::yield_context yield) {
            httpGet_(
                HOSTNAME, PORT, target,
                [&promise](const std::string &response_) {
                    promise.set_value(response_);
                },
                yield);
        });
    (void)spawn_op;
}

DataProvider::~DataProvider() {}
} // namespace sravz::services::data_provider
