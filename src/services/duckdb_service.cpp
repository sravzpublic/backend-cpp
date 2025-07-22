// Sravz LLC
#include <aws/core/Aws.h>

#include <aws_s3_service.hpp>
#include <boost/functional/hash.hpp>
#include <cstdlib>
#include <duckdb_service.hpp>
#include <iostream>
#include <utils.hpp>

namespace sravz::services::duck_db {

DuckDBClient::DuckDBClient(
    std::shared_ptr<sravz::services::aws::s3::S3Client> &s3ClientContabo)
    : s3ClientContabo_(s3ClientContabo) {
    setenv("DUCKDB_EXTENSION_ALLOW_UNSIGNED", "1", 1);
    duckdb::DuckDB db(nullptr);
    _db = std::make_unique<duckdb::DuckDB>(db);
    _duckdb_connection = std::make_unique<duckdb::Connection>(db);
    // _duckdb_connection->Query("INSTALL httpfs;");
    _duckdb_connection->Query("LOAD httpfs;");
    // _duckdb_connection->Query("INSTALL json;");
    _duckdb_connection->Query("LOAD json;");
    // _duckdb_connection->Query("SET autoinstall_known_extensions=1;");
    // _duckdb_connection->Query("SET autoload_known_extensions=1;");
    auto result = _duckdb_connection->Query("SELECT duckdb_version();");
    result->Print();
    result = _duckdb_connection->Query("PRAGMA show_extensions;");
    result->Print();
}

bool DuckDBClient::table_exists(const std::string &table_name) {
    auto result = _duckdb_connection->Query(
        "SELECT EXISTS (SELECT 1 FROM information_schema.tables WHERE "
        "table_name = ?)",
        table_name);

    // Fetch data chunks
    while (auto chunk = result->Fetch()) {
        // Process the chunk
        for (size_t row_idx = 0; row_idx < chunk->size(); ++row_idx) {
            for (size_t col_idx = 0; col_idx < chunk->ColumnCount();
                 ++col_idx) {
                // Access data from the chunk
                auto value =
                    chunk->GetValue(col_idx, row_idx).GetValue<int32_t>();
                std::cout << "Value: " << value << std::endl;
                return value == 1; // Check if the table exists
                // Add more type cases as necessary
            }
        }
    }
    return false; // In case of error or if the result doesn't contain any value
}

boost::json::array
DuckDBClient::select_from_s3_parquet_file(const std::string &bucket_name,
                                          const std::string &prefix,
                                          const std::string &where_clause) {
    std::size_t bucket_hash = boost::hash_value(bucket_name);
    std::size_t prefix_hash = boost::hash_value(prefix);
    std::string table_name = "sravz_" + std::to_string(bucket_hash) + "_" +
        std::to_string(prefix_hash);

    std::ostringstream oss;
    oss << "CREATE TABLE " << table_name
        << " AS SELECT * FROM parquet_scan('presigned_url_to_replace')";
    CreateTableIfDoesNotExists(bucket_name, prefix, table_name, oss.str());

    auto result2 =
        _duckdb_connection->Query("SELECT to_json(" + table_name + ") FROM " +
                                  table_name + " WHERE " + where_clause);

    boost::json::array json_array;
    for (idx_t i = 0; i < result2->Collection().Count(); ++i) {
        duckdb::Value json_value = result2->GetValue(0, i);
        std::string json_string = json_value.ToString();
        std::cout << json_string << std::endl;
        json_array.push_back(std::move(boost::json::parse(json_string)));
    }

    return json_array;
}

void DuckDBClient::CreateTableIfDoesNotExists(
    const std::string &bucket_name, const std::string &prefix,
    const std::string &table_name, const std::string &create_statement) {
    if (!table_exists(table_name)) {
        auto presigned_url =
            s3ClientContabo_->generatePresignedUrl(bucket_name, prefix);
        // Replace "presigned_url_to_replace" with the actual presigned URL
        std::string updated_create_statement = create_statement;
        size_t pos = updated_create_statement.find("presigned_url_to_replace");
        if (pos != std::string::npos) {
            updated_create_statement.replace(
                pos, std::string("presigned_url_to_replace").length(),
                presigned_url);
        }

        // Execute the updated create statement
        _duckdb_connection->Query("SET autoinstall_known_extensions=1;");
        _duckdb_connection->Query("SET autoload_known_extensions=1;");

        auto query_result = _duckdb_connection->Query(updated_create_statement);
        if (query_result->HasError()) {
            std::cerr << "DuckDB CREATE TABLE error: "
                      << query_result->GetError() << std::endl;
        }

        if (!table_exists(table_name)) {
            std::cerr << "Table " << table_name
                      << " was not created successfully." << std::endl;
        }
    }
}

DuckDBClient::~DuckDBClient() {}
} // namespace sravz::services::duck_db
