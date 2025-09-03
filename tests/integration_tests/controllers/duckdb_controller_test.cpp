#include <gtest/gtest.h>

#include <aws_s3_service.hpp>
#include <duckdb_controller.hpp>
#include <duckdb_model.hpp>
#include <duckdb_service.hpp>

TEST(duckdb_controller, Select) {
    Aws::SDKOptions const options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto duckdbClient_ =
            std::make_shared<sravz::services::duck_db::DuckDBClient>(
                s3ClientContabo_);
        auto duckDBController_ =
            std::make_shared<sravz::controllers::duck_db::DuckDBController>(
                duckdbClient_);

        std::cout << duckDBController_->select_from_s3_parquet_file(
            "sravz", "sravz-production/price_stats/quotes_stats_latest.parquet",
            "sravz_id='stk_us_nvda'");
    }
    Aws::ShutdownAPI(options);
}

TEST(duckdb_controller, SelectIn) {
    Aws::SDKOptions const options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto duckdbClient_ =
            std::make_shared<sravz::services::duck_db::DuckDBClient>(
                s3ClientContabo_);
        auto duckDBController_ =
            std::make_shared<sravz::controllers::duck_db::DuckDBController>(
                duckdbClient_);

        std::cout << duckDBController_->select_from_s3_parquet_file(
            "sravz", "sravz-production/price_stats/quotes_stats_latest.parquet",
            "sravz_id in ('stk_us_nvda', 'stk_us_aapl', 'stk_us_amzn')");
    }
    Aws::ShutdownAPI(options);
}

TEST(duckdb_controller, Get) {
    Aws::SDKOptions const options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        auto duckdbClient_ =
            std::make_shared<sravz::services::duck_db::DuckDBClient>(
                s3ClientContabo_);
        auto duckDBController_ =
            std::make_shared<sravz::controllers::duck_db::DuckDBController>(
                duckdbClient_);

        sravz::models::DuckDBModel model;
        model.set_bucket("sravz");
        model.set_key(
            "sravz-production/price_stats/quotes_stats_latest.parquet");
        model.set_where_clause("sravz_id='stk_us_nvda'");
        model = duckDBController_->get(model);
        std::cout << model.get_db_query_result();
        ASSERT_NE(model.get_db_query_result().find("annual_return"),
                  std::string::npos);
    }
    Aws::ShutdownAPI(options);
}