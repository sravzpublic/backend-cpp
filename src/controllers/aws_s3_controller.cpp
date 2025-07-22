#include <aws_s3_controller.hpp>
#include <boost/shared_ptr.hpp>

namespace sravz::controllers::aws {
std::string AWSS3Controller::get(const std::string &bucket,
                                 const std::string &key,
                                 const std::string &column,
                                 const std::string &in_clause) {
    // return s3Client_->s3select("sravz-data",
    // "historical/ytd_us.parquet", "Code", "TQQQ,QLD,QQQ");
    return s3Client_->s3select(bucket, key, column, in_clause);
    // sravz::services::aws::s3::S3Client s3client("us-east-1",
    // "sravz-data", std::move(options),
    // sravz::services::aws::s3::S3ClientType::AWS); std::cout <<
    // s3client.s3select("sravz-data", "historical/ytd_us.parquet",
    // "Code", "TQQQ,QLD,QQQ") << std::endl;
}
} // namespace sravz::controllers::aws
