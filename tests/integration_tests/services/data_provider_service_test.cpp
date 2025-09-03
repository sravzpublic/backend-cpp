#include <gtest/gtest.h>

#include <boost/asio.hpp>

namespace asio = boost::asio;

#include "data_provider_service.hpp"

TEST(data_provider_service_callback, integration) {
    asio::io_context io_context;
    const std::string host = "eodhd.com";
    const std::string target =
        "/api/eod/MCD.US?period=d&api_token=demo&fmt=json";
    auto dataProvider =
        sravz::services::data_provider::DataProvider(io_context);
    std::string response;
    dataProvider.httpGet(
        host, "443", target,
        [&response](const std::string &response_) { response = response_; });
    io_context.run();
    ASSERT_FALSE(response.empty());
}

TEST(data_provider_service_promise, integration) {
    asio::io_context io_context;
    const std::string host = "eodhd.com";
    const std::string target =
        "/api/eod/MCD.US?period=d&api_token=demo&fmt=json";
    auto dataProvider =
        sravz::services::data_provider::DataProvider(io_context);
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();
    dataProvider.httpGet(host, "443", target, promise);
    io_context.run();
    ASSERT_FALSE(future.get().empty());
    ;
}