#pragma once
#include <aws/core/Aws.h>

#include <aws_s3_service.hpp>
#include <boost/json.hpp>
#include <future>
#include <memory>
#include <redis_service.hpp>

#include "base_controller.hpp"
#include "data_provider_service.hpp"
#include "presigned_url_model.hpp"

namespace sravz::controllers::fundamentals {
class FundamentalsController : public sravz::controller::BaseController {
public:
    FundamentalsController(
        std::shared_ptr<sravz::services::aws::s3::S3Client> &s3Client,
        std::shared_ptr<sravz::services::redis::RedisClient> &redisClient,
        std::shared_ptr<sravz::services::data_provider::DataProvider>
            &dataProvider)
        : s3Client_(s3Client),
          redisClient_(redisClient),
          dataProvider_(dataProvider) {}
    void CheckAndFetchData(const std::string &sravzID,
                           std::string &completeFileLocation);
    void FetchDataAndSaveToS3(const std::string &sravzID,
                              std::string &completeFileLocation);
    static constexpr const char *FUNDAMENTALS_BUCKET_NAME = "sravz-data";
    static constexpr const char *FUNDAMENTALS_PREFIX =
        "/mutual_funds/fundamentals/";
    std::chrono::seconds KEY_EXPIRY_IN_SECONDS = std::chrono::hours(24);
    sravz::models::PreSignedUrlModel &
    get(sravz::models::PreSignedUrlModel &preSignedUrlModel);
    sravz::models::PreSignedUrlModel &
    validate_request(sravz::models::PreSignedUrlModel &preSignedUrlModel);

private:
    std::shared_ptr<sravz::services::aws::s3::S3Client> s3Client_;
    std::shared_ptr<sravz::services::redis::RedisClient> redisClient_;
    std::shared_ptr<sravz::services::data_provider::DataProvider> dataProvider_;
    Aws::SDKOptions awsOptions_;
};
} // namespace sravz::controllers::fundamentals