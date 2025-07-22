#pragma once
// Service includes
#include <aws_s3_service.hpp>
#include <mongo_service.hpp>
#include <redis_service.hpp>
#include <duckdb_service.hpp>

#include "data_provider_service.hpp"

// Controller includes
#include <aws_s3_controller.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <boost/url.hpp>
#include <fundamentals_controller.hpp>
#include <mongo_controller.hpp>
#include <duckdb_controller.hpp>
// Models
#include "base_controller.hpp"
#include "base_model.hpp"
#include "presigned_url_model.hpp"

namespace sravz {
class router {
public:
    router(net::io_context &ioc_background_worker)
        : ioc_background_worker_(ioc_background_worker) {
        init_();
    }
    std::pair<boost::beast::http::status, std::string>
    route(boost::beast::http::verb method, const std::string &uri,
          const std::string &body);

    template <typename T1, typename T2>
    const std::pair<boost::beast::http::status, std::string>
    route_to_controller(std::map<std::string, std::string> &params_map,
                        T2 &controller);

private:
    void init_();
    // Services
    std::shared_ptr<sravz::services::mongo::MongoClient> mongoClient_;
    std::shared_ptr<sravz::services::aws::s3::S3Client> s3Client_;
    std::shared_ptr<sravz::services::aws::s3::S3Client> s3ClientContabo_;
    std::shared_ptr<sravz::services::redis::RedisClient> redisClient_;
    std::shared_ptr<sravz::services::data_provider::DataProvider> dataProvider_;
    std::shared_ptr<sravz::services::duck_db::DuckDBClient> duckdbClient_;
    // Controllers
    std::shared_ptr<sravz::controllers::mongo::MongoController>
        mongoController_;
    std::shared_ptr<sravz::controllers::duck_db::DuckDBController>
        duckDBController_; 
    std::shared_ptr<sravz::controllers::aws::AWSS3Controller> awsS3Controller_;
    std::shared_ptr<sravz::controllers::fundamentals::FundamentalsController>
        fundamentalsController_;

    net::io_context &ioc_background_worker_;
};
} // namespace sravz
