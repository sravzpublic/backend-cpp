#include <gtest/gtest.h>

#include <aws_s3_service.hpp>
#include <duckdb_service.hpp>
using namespace std;

TEST(duckdb_service, test_table_create) {
    Aws::SDKOptions options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        sravz::services::duck_db::DuckDBClient duckDBClient(s3ClientContabo_);
        EXPECT_EQ(duckDBClient.table_exists("test_table"), false);
        std::string fake_string = "fake";
        std::string table_name = "fake";
        std::string create_statement =
            "CREATE TABLE fake (fake_string VARCHAR)";
        duckDBClient.CreateTableIfDoesNotExists(fake_string, fake_string,
                                                table_name, create_statement);
        EXPECT_EQ(duckDBClient.table_exists(table_name), true);
    }
    Aws::ShutdownAPI(options);
}

TEST(duckdb_service, select_from_s3_parquet_file) {
    Aws::SDKOptions options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        sravz::services::duck_db::DuckDBClient duckDBClient(s3ClientContabo_);
        duckDBClient.select_from_s3_parquet_file(
            "sravz", "sravz-production/price_stats/quotes_stats_latest.parquet",
            "sravz_id='stk_us_nvda'");
    }
    Aws::ShutdownAPI(options);
}

TEST(duckdb_service, select_from_s3_parquet_file_1) {
    Aws::SDKOptions options;

    Aws::InitAPI(options);
    {
        auto s3ClientContabo_ =
            std::make_shared<sravz::services::aws::s3::S3Client>(
                "us-east-1", std::move(options),
                sravz::services::aws::s3::S3ClientType::Contabo);
        sravz::services::duck_db::DuckDBClient duckDBClient(s3ClientContabo_);
        duckDBClient.select_from_s3_parquet_file(
            "sravz-data", "historical/quotes_summary_stats_test.parquet",
            "sravz_id='stk_us_mmm'");
    }
    Aws::ShutdownAPI(options);
}