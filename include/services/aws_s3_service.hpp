#pragma once
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <string>
#include <vector>

namespace sravz::services::aws::s3 {
enum class S3ClientType { Contabo = 1, AWS = 2 };

class S3Client {
public:
    S3Client(std::string region, Aws::SDKOptions options, S3ClientType type);
    bool putObject(const std::string &bucket, const std::string &key,
                   const std::string &value);
    bool putObjectGzip(const std::string &bucketName,
                       const std::string &objectKey, const std::string &data);
    std::string getObject(const std::string &bucket, const std::string &key);
    std::vector<std::string> listObjects(const std::string &bucket);
    bool deleteObject(const std::string &bucket, const std::string &key);
    bool objectExists(const std::string &bucket, const std::string &key);
    std::string s3select(const std::string &bucket, const std::string &key,
                         const std::string &column, const std::string &value);
    std::string generatePresignedUrl(const std::string &bucketName,
                                     const std::string &objectKey);
    ~S3Client() = default;

private:
    std::string m_region;
    std::string m_bucket;
    Aws::Client::ClientConfiguration m_clientConfig;
    Aws::S3::S3Client m_s3Client;
    Aws::SDKOptions options_;
    S3ClientType type_;
};
} // namespace sravz::services::aws::s3