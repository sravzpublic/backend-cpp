#include <aws/core/Aws.h>
#include <gtest/gtest.h>

#include <aws_s3_service.hpp>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <fundamentals_controller.hpp>
#include <iostream>
#include <redis_service.hpp>

#include "data_provider_service.hpp"

namespace asio = boost::asio;

TEST(fundamentals_controller, GeneratePresignedURL) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        std::string prefix =
            "https://usc1.contabostorage.com/sravz-data/mutual_funds/"
            "fundamentals/commonstock_us_arkvx.json1";
        std::string sravzID = "commonstock_us_arkvx.json1";
        std::ostringstream oss;
        oss << "https://usc1.contabostorage.com/"
            << sravz::controllers::fundamentals::FundamentalsController::
                   FUNDAMENTALS_BUCKET_NAME
            << sravz::controllers::fundamentals::FundamentalsController::
                   FUNDAMENTALS_PREFIX
            << sravzID;
        std::string completeFileLocation = oss.str();

        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto redisClient_ =
            std::make_shared<sravz::services::redis::RedisClient>();
        asio::io_context io_context;
        auto dataProvider_ =
            std::make_shared<sravz::services::data_provider::DataProvider>(
                io_context);

        auto fundamentalsController_ = std::make_shared<
            sravz::controllers::fundamentals::FundamentalsController>(
            s3ClientContabo_, redisClient_, dataProvider_);

        // Create executor_work_guard to prevent io_context from stopping
        auto workGuard = boost::asio::make_work_guard(io_context);
        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        int threads = 3;
        v.reserve(threads);
        for (auto i = threads - 1; i > 0; --i)
            v.emplace_back([&io_context] {
                BOOST_LOG_TRIVIAL(info) << "Starting ioc";
                io_context.run();
            });

        std::map<std::string, std::string> params_map = {
            {"sravzid", "commonstock_us_arkvx.json1"}};

        auto model = sravz::models::PreSignedUrlModel(params_map);
        model = fundamentalsController_->validate_request(model);
        model = fundamentalsController_->get(model);

        ASSERT_FALSE(model.get_url().empty());
        ASSERT_TRUE(model.get_url().find(prefix) == 0)
            << "String doesn't start with prefix";

        io_context.stop();

        for (auto &t : v)
            t.join();
    }
    Aws::ShutdownAPI(options);
}

TEST(fundamentals_controller, FetchDataAndSaveToS3) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        std::string prefix =
            "https://usc1.contabostorage.com/sravz-data/mutual_funds/"
            "fundamentals/commonstock_us_arkvx.json1";
        std::string sravzID = "commonstock_us_arkvx.json1";
        std::ostringstream oss;
        oss << "https://usc1.contabostorage.com/"
            << sravz::controllers::fundamentals::FundamentalsController::
                   FUNDAMENTALS_BUCKET_NAME
            << "/test-mutual-funds-fundamentals/" << sravzID;
        std::string completeFileLocation = oss.str();

        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto redisClient_ =
            std::make_shared<sravz::services::redis::RedisClient>();
        asio::io_context io_context;
        auto dataProvider_ =
            std::make_shared<sravz::services::data_provider::DataProvider>(
                io_context);

        auto fundamentalsController_ = std::make_shared<
            sravz::controllers::fundamentals::FundamentalsController>(
            s3ClientContabo_, redisClient_, dataProvider_);

        // Create executor_work_guard to prevent io_context from stopping
        auto workGuard = boost::asio::make_work_guard(io_context);
        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        int threads = 3;
        v.reserve(threads);
        for (auto i = threads - 1; i > 0; --i)
            v.emplace_back([&io_context] {
                BOOST_LOG_TRIVIAL(info) << "Starting ioc";
                io_context.run();
            });

        s3ClientContabo_->deleteObject(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation);

        ASSERT_FALSE(s3ClientContabo_->objectExists(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation));

        fundamentalsController_->FetchDataAndSaveToS3(sravzID,
                                                      completeFileLocation);

        ASSERT_TRUE(s3ClientContabo_->objectExists(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation));

        s3ClientContabo_->deleteObject(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation);

        io_context.stop();

        for (auto &t : v)
            t.join();
    }
    Aws::ShutdownAPI(options);
}

TEST(fundamentals_controller, CheckAndFetchData) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        std::string prefix =
            "https://usc1.contabostorage.com/sravz-data/mutual_funds/"
            "fundamentals/commonstock_us_arkvx.json1";
        std::string sravzID = "commonstock_us_arkvx.json1";
        std::ostringstream oss;
        oss << "https://usc1.contabostorage.com/"
            << sravz::controllers::fundamentals::FundamentalsController::
                   FUNDAMENTALS_BUCKET_NAME
            << "/test-mutual-funds-fundamentals/" << sravzID;
        std::string completeFileLocation = oss.str();

        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto redisClient_ =
            std::make_shared<sravz::services::redis::RedisClient>();
        asio::io_context io_context;
        auto dataProvider_ =
            std::make_shared<sravz::services::data_provider::DataProvider>(
                io_context);

        auto fundamentalsController_ = std::make_shared<
            sravz::controllers::fundamentals::FundamentalsController>(
            s3ClientContabo_, redisClient_, dataProvider_);

        // Create executor_work_guard to prevent io_context from stopping
        auto workGuard = boost::asio::make_work_guard(io_context);
        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        int threads = 3;
        v.reserve(threads);
        for (auto i = threads - 1; i > 0; --i)
            v.emplace_back([&io_context] {
                BOOST_LOG_TRIVIAL(info) << "Starting ioc";
                io_context.run();
            });

        s3ClientContabo_->deleteObject(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation);

        ASSERT_FALSE(s3ClientContabo_->objectExists(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation));

        fundamentalsController_->CheckAndFetchData(sravzID,
                                                   completeFileLocation);

        ASSERT_TRUE(s3ClientContabo_->objectExists(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation));

        s3ClientContabo_->deleteObject(
            sravz::controllers::fundamentals::FundamentalsController::
                FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation);

        io_context.stop();

        for (auto &t : v)
            t.join();
    }
    Aws::ShutdownAPI(options);
}