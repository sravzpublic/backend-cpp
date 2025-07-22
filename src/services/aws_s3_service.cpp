#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/signer/AWSAuthV4Signer.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/http/HttpTypes.h>
#include <aws/core/http/Scheme.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/ExpressionType.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectResult.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/InputSerialization.h>
#include <aws/s3/model/JSONOutput.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/OutputSerialization.h>
#include <aws/s3/model/ParquetInput.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/PutObjectResult.h>
#include <aws/s3/model/RecordsEvent.h>
#include <aws/s3/model/SelectObjectContentHandler.h>
#include <aws/s3/model/SelectObjectContentRequest.h>

#include <aws_s3_service.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <utils.hpp>
#include <vector>

namespace sravz::services::aws::s3 {
S3Client::S3Client(std::string region, Aws::SDKOptions options,
                   S3ClientType type)
    : m_region(std::move(region)),
      options_(std::move(std::move(options))),
      type_(type) {
    Aws::Client::ClientConfiguration client_config;
    switch (type) {
        case S3ClientType::Contabo: {
            client_config.endpointOverride =
                Aws::String("https://usc1.contabostorage.com/");
            client_config.scheme = Aws::Http::Scheme::HTTPS;
            std::string contabo_key;
            std::string contabo_secret;
            sravz::utils::Getenv("CONTABO_KEY", contabo_key);
            sravz::utils::Getenv("CONTABO_SECRET", contabo_secret);
            Aws::Auth::AWSCredentials const credentials_contabo(contabo_key,
                                                                contabo_secret);
            m_s3Client = Aws::S3::S3Client(
                credentials_contabo, client_config,
                Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never,
                false);
            break;
        }
        case S3ClientType::AWS: {
            std::string aws_access_key_id;
            std::string aws_secret_access_key;
            sravz::utils::Getenv("AWS_ACCESS_KEY_ID", aws_access_key_id);
            sravz::utils::Getenv("AWS_SECRET_ACCESS_KEY",
                                 aws_secret_access_key);
            Aws::Auth::AWSCredentials const credentials_aws(
                aws_access_key_id, aws_secret_access_key);
            m_s3Client = Aws::S3::S3Client(credentials_aws);
            break;
        }
        default:
            BOOST_LOG_TRIVIAL(info) << "Unsupported message type";
            break;
    }
}

auto S3Client::putObject(const std::string &bucket, const std::string &key,
                         const std::string &value) -> bool {
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);
    // Creates a shared pointer of Aws::StringStream
    auto stream = Aws::MakeShared<Aws::StringStream>("putObject", "");
    *stream << value;
    request.SetBody(stream);

    auto outcome = m_s3Client.PutObject(request);
    if (outcome.IsSuccess()) {
        return true;
    }
    BOOST_LOG_TRIVIAL(error) << "Failed with error: " << outcome.GetError();
    return false;
}

// GZip enconding does not work with Contabo. Works with AWS S3
auto S3Client::putObjectGzip(const std::string &bucketName,
                             const std::string &objectKey,
                             const std::string &data) -> bool {
    Aws::Client::ClientConfiguration config;
    Aws::S3::S3Client s3_client(config);

    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectKey);

    // Set content type to gzip
    request.SetContentEncoding("gzip");
    request.SetContentType("application/json");

    // Convert compressed data to string
    std::string compressedDataStr = sravz::utils::CompressData(data);

    // Set the compressed data as the body of the request
    auto bodyStream = Aws::MakeShared<Aws::StringStream>("BodyStream");
    *bodyStream << compressedDataStr;
    request.SetBody(bodyStream);

    // Upload the object to S3
    auto outcome = s3_client.PutObject(request);
    if (!outcome.IsSuccess()) {
        BOOST_LOG_TRIVIAL(error)
            << "Failed with error: " << outcome.GetError().GetMessage();
        return false;
    }
    return true;
}

auto S3Client::getObject(const std::string &bucket, const std::string &key)
    -> std::string {
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);

    auto outcome = m_s3Client.GetObject(request);
    if (outcome.IsSuccess()) {
        // Takes ownership of the GetObjectResult
        auto &retrieved_object = outcome.GetResultWithOwnership().GetBody();
        std::stringstream string_stream_buffer;
        string_stream_buffer << retrieved_object.rdbuf();
        return string_stream_buffer.str();
    }
    BOOST_LOG_TRIVIAL(info) << "Failed with error: " << outcome.GetError();
    return "";
}

auto S3Client::listObjects(const std::string &bucket)
    -> std::vector<std::string> {
    Aws::S3::Model::ListObjectsRequest request;
    request.SetBucket(bucket);

    std::vector<std::string> object_keys;
    auto outcome = m_s3Client.ListObjects(request);
    if (outcome.IsSuccess()) {
        auto objects = outcome.GetResult().GetContents();
        for (const auto &object : objects) {
            object_keys.push_back(object.GetKey());
        }
    } else {
        BOOST_LOG_TRIVIAL(info) << "Failed with error: " << outcome.GetError();
    }
    return object_keys;
}

auto S3Client::deleteObject(const std::string &bucket, const std::string &key)
    -> bool {
    Aws::S3::Model::DeleteObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);

    auto outcome = m_s3Client.DeleteObject(request);
    if (outcome.IsSuccess()) {
        return true;
    }
    BOOST_LOG_TRIVIAL(info) << "Failed with error: " << outcome.GetError();
    return false;
}

auto S3Client::objectExists(const std::string &bucket, const std::string &key)
    -> bool {
    Aws::S3::Model::HeadObjectRequest head_object_request;
    head_object_request.WithBucket(bucket).WithKey(key);

    bool object_exists = false;
    auto outcome = m_s3Client.HeadObject(head_object_request);
    if (outcome.IsSuccess()) {
        object_exists = true;
        BOOST_LOG_TRIVIAL(info)
            << "Object '" << key << "' exists in bucket '" << bucket << "'";
    } else {
        BOOST_LOG_TRIVIAL(info)
            << "Object '" << key << "' does not exist in bucket '" << bucket
            << "'";
    }
    return object_exists;
}

auto S3Client::s3select(const std::string &bucket, const std::string &key,
                        const std::string &column, const std::string &in_clause)
    -> std::string {
    Aws::S3::Model::SelectObjectContentRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);
    request.SetExpressionType(Aws::S3::Model::ExpressionType::SQL);
    std::ostringstream buffer;
    buffer << "[";

    if (!in_clause.empty()) {
        std::vector<std::string> params;
        std::stringstream string_stream_buffer(in_clause);
        std::string param;
        while (std::getline(string_stream_buffer, param, ',')) {
            params.push_back(param);
        }
        std::string in_clause = "(";
        for (auto it = params.begin(); it != params.end(); ++it) {
            in_clause += "'" + *it + "'";
            if (it != params.end() - 1) {
                in_clause += ",";
            }
        }
        in_clause += ")";
        BOOST_LOG_TRIVIAL(info)
            << "Select statement: SELECT * FROM S3Object WHERE " << column
            << " IN " << in_clause;
        request.SetExpression("SELECT * FROM S3Object WHERE " + column +
                              " IN " + in_clause);
    } else {
        request.SetExpression("SELECT * FROM S3Object");
    }

    // Set the input and output serialization formats for Parquet
    Aws::S3::Model::InputSerialization input_serialization;
    input_serialization.SetParquet(Aws::S3::Model::ParquetInput());
    Aws::S3::Model::OutputSerialization output_serialization;
    Aws::S3::Model::JSONOutput json_output;
    json_output.SetRecordDelimiter(",");
    output_serialization.SetJSON(json_output);
    request.SetInputSerialization(input_serialization);
    request.SetOutputSerialization(output_serialization);

    Aws::S3::Model::SelectObjectContentHandler handler;
    handler.SetRecordsEventCallback(
        [&buffer](const Aws::S3::Model::RecordsEvent &recordsEvent) {
            auto records_vector = recordsEvent.GetPayload();
            Aws::String const records(records_vector.begin(),
                                      records_vector.end());
            buffer << records.c_str();
        });
    // handler.SetStatsEventCallback(
    //     [=](const Aws::S3::Model::StatsEvent &statsEvent) {

    //     });

    // Register a callback function to handle the S3 Select events
    request.SetEventStreamHandler(handler);

    // Issue the S3 Select request
    auto outcome = m_s3Client.SelectObjectContent(request);
    if (outcome.IsSuccess()) {
        BOOST_LOG_TRIVIAL(info) << "S3 Select request succeeded";
    } else {
        BOOST_LOG_TRIVIAL(info)
            << "S3 Select request failed: " << outcome.GetError().GetMessage();
    }
    auto output_string = buffer.str();
    if (!output_string.empty() && output_string.back() == ',') {
        output_string.pop_back();
    }
    output_string += "]";
    return output_string;
}

auto S3Client::generatePresignedUrl(const std::string &bucketName,
                                    const std::string &objectKey)
    -> std::string {
    return m_s3Client.GeneratePresignedUrl(bucketName, objectKey,
                                           Aws::Http::HttpMethod::HTTP_GET);
}

// S3Client::~S3Client() {
//     // Aws::ShutdownAPI(options_);
// }

} // namespace sravz::services::aws::s3
