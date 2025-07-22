#pragma once
#include <aws_s3_service.hpp>
#include <boost/json.hpp>
#include <duckdb_service.hpp>

#include "duckdb.hpp"

namespace sravz::services::duck_db {

class DuckDBClient {
public:
    DuckDBClient(
        std::shared_ptr<sravz::services::aws::s3::S3Client> &s3ClientContabo);
    bool table_exists(const std::string &table_name);

    ~DuckDBClient();
    boost::json::array select_from_s3_parquet_file(const std::string &bucket_name,
                       const std::string &prefix,
                       const std::string &where_clause);

    void CreateTableIfDoesNotExists(const std::string &bucket_name,
                                    const std::string &prefix,
                                    const std::string &table_name,
                                    const std::string &create_statement);

private:
    std::unique_ptr<duckdb::DuckDB> _db = nullptr;
    std::unique_ptr<duckdb::Connection> _duckdb_connection = nullptr;
    std::shared_ptr<sravz::services::aws::s3::S3Client> s3ClientContabo_;
};
} // namespace sravz::services::duck_db
