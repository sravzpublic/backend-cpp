#include <duckdb_controller.hpp>

namespace sravz {
namespace controllers {
    namespace duck_db {
        boost::json::array DuckDBController::select_from_s3_parquet_file(
            const std::string &bucket_name, const std::string &prefix,
            const std::string &where_clause) {
            return duckdbClient_->select_from_s3_parquet_file(
                bucket_name, prefix, where_clause);
        }

        sravz::models::DuckDBModel &DuckDBController::get(
            sravz::models::DuckDBModel &DuckDBModel) { // override {
            DuckDBModel.set_db_query_result(select_from_s3_parquet_file(
                DuckDBModel.get_bucket(), DuckDBModel.get_key(),
                DuckDBModel.get_where_clause()));
            return DuckDBModel;
        }

        sravz::models::DuckDBModel &DuckDBController::validate_request(
            sravz::models::DuckDBModel &duckDBModel) {
            duckDBModel.set_param("bucket",
                                  [&duckDBModel](const std::string &v) {
                                      duckDBModel.set_bucket(v);
                                  });
            duckDBModel.set_param("key", [&duckDBModel](const std::string &v) {
                duckDBModel.set_key(v);
            });
            duckDBModel.set_param("where_clause",
                                  [&duckDBModel](const std::string &v) {
                                      duckDBModel.set_where_clause(v);
                                  });
            return duckDBModel;
        }

    } // namespace duck_db
} // namespace controllers
} // namespace sravz