#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <aws_s3_service.hpp>
#include <iostream>

int main() {
    Aws::SDKOptions options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", "sravz-data", std::move(options),
            sravz::services::aws::s3::S3ClientType::CONTABO);

        // Use the sravz::services::aws:s3::S3Client object to put an object.
        if (s3client.putObject("test-key", "Hello, world!")) {
            std::cout << "Object put successfully." << std::endl;
        } else {
            std::cout << "Object put failed." << std::endl;
        }

        // Use the sravz::services::aws:s3::S3Client object to get an object.
        std::string objectValue = s3client.getObject("test-key");
        std::cout << "Retrieved object value: " << objectValue << std::endl;

        // Use the sravz::services::aws:s3::S3Client object to list objects in
        // the bucket.
        std::vector<std::string> objectKeys = s3client.listObjects();
        std::cout << "Object keys in bucket: ";
        for (const auto &key : objectKeys) {
            std::cout << key << ", ";
        }
        std::cout << std::endl;
        // Use the sravz::services::aws:s3::S3Client object to delete an object.
        if (s3client.deleteObject("test-key")) {
            std::cout << "Object deleted successfully." << std::endl;
        } else {
            std::cout << "Object delete failed." << std::endl;
        }
    }
    Aws::ShutdownAPI(options);

    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", "sravz-data", std::move(options),
            sravz::services::aws::s3::S3ClientType::AWS);
        std::cout << s3client.s3select("sravz-data",
                                       "historical/ytd_us.parquet", "Code",
                                       "TQQQ,QLD,QQQ")
                  << std::endl;
    }
    Aws::ShutdownAPI(options);

    return 0;
}
