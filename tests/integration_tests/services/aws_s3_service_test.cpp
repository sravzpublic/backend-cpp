#include <aws/core/Aws.h>
#include <gtest/gtest.h>

#include <aws_s3_service.hpp>

TEST(aws_s3_service, put_get_list_delete_object) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", options,
            sravz::services::aws::s3::S3ClientType::Contabo);

        // Put Object, get object, list objects and delete object
        ASSERT_TRUE(
            s3client.putObject("sravz-data", "test-key", "Hello, world!"));

        ASSERT_TRUE(s3client.objectExists("sravz-data", "test-key"));
        ASSERT_FALSE(s3client.objectExists("sravz-data", "test-key1"));

        ASSERT_FALSE(s3client.getObject("sravz-data", "test-key").empty());

        ASSERT_FALSE(s3client.listObjects("sravz-data").empty());

        // ASSERT_TRUE(s3client.deleteObject("sravz-data", "test-key"));
    }
    Aws::ShutdownAPI(options);
}

TEST(aws_s3_service, gz_put_get_delete_object) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", options,
            sravz::services::aws::s3::S3ClientType::Contabo);

        // Put GZip Object, Get GZip Object Decompressed, Delete Object
        ASSERT_TRUE(s3client.putObjectGzip("sravz-data", "test-keygz",
                                           R"(
                                                {
                                                    "name": "John Doe",
                                                    "age": 30,
                                                    "city": "New York"
                                                }
                                            )"));
        // ASSERT_TRUE(s3client.objectExists("sravz-data", "test-keygz"));
        // ASSERT_FALSE(s3client.getObject("sravz-data",
        // "test-key-gz").empty());
    }
    Aws::ShutdownAPI(options);
}

TEST(aws_s3_service, pre_signed_url) {
    Aws::SDKOptions const options;
    // options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", options,
            sravz::services::aws::s3::S3ClientType::Contabo);

        // PreSignedURLs
        ASSERT_FALSE(
            s3client.generatePresignedUrl("sravz-data", "test-key").empty());
    }
    Aws::ShutdownAPI(options);
}

TEST(aws_s3_service, s3_select) {
    Aws::SDKOptions const options;

    Aws::InitAPI(options);
    {
        sravz::services::aws::s3::S3Client s3client(
            "us-east-1", options, sravz::services::aws::s3::S3ClientType::AWS);
        ASSERT_FALSE(s3client
                         .s3select("sravz-data", "historical/ytd_us.parquet",
                                   "Code", "TQQQ,QLD,QQQ")
                         .empty());
    }
    Aws::ShutdownAPI(options);
}