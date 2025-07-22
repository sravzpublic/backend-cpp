#pragma once
#include <boost/log/trivial.hpp>
#include <duckdb_model.hpp>
#include <duckdb_service.hpp>
#include <memory>

#include "base_controller.hpp"

namespace sravz {
namespace controllers {
    namespace duck_db {
        class DuckDBController : public sravz::controller::BaseController {
        public:
            DuckDBController(
                std::shared_ptr<sravz::services::duck_db::DuckDBClient>
                    &duckdbClient)
                : duckdbClient_(duckdbClient) {}
            boost::json::array
            select_from_s3_parquet_file(const std::string &bucket_name,
                                        const std::string &prefix,
                                        const std::string &where_clause);
            sravz::models::DuckDBModel &
            get(sravz::models::DuckDBModel &duckDBModel);
            sravz::models::DuckDBModel &
            validate_request(sravz::models::DuckDBModel &duckDBModel);

        private:
            std::shared_ptr<sravz::services::duck_db::DuckDBClient>
                duckdbClient_;
        };
    } // namespace duck_db
} // namespace controllers
} // namespace sravz