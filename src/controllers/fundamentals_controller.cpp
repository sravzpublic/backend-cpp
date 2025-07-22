#include <fundamentals_controller.hpp>

namespace sravz::controllers::fundamentals {
/// @brief For the Given Sravz ID lookup or generate and cache presigned URL
/// @param sravzID
/// @return PreSignedURL
sravz::models::PreSignedUrlModel &FundamentalsController::get(
    sravz::models::PreSignedUrlModel &preSignedUrlModel) { // override {
    std::ostringstream oss;
    oss << FundamentalsController::FUNDAMENTALS_PREFIX
        << preSignedUrlModel.get_sravzid() << ".json";
    std::string completeFileLocation = oss.str();

    std::optional<std::string> redisPresignedURL =
        redisClient_->get(completeFileLocation);
    if (redisPresignedURL.has_value()) {
        preSignedUrlModel.set_url(redisPresignedURL.value());
    } else {
        CheckAndFetchData(preSignedUrlModel.get_sravzid(),
                          completeFileLocation);
        auto preSignedURL = s3Client_->generatePresignedUrl(
            FundamentalsController::FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation);
        redisClient_->set_with_expiry(completeFileLocation, preSignedURL,
                                      KEY_EXPIRY_IN_SECONDS);
        preSignedUrlModel.set_url(preSignedURL);
    }
    return preSignedUrlModel;
}
/// @brief Check if data for the SravzID exists. If Not fetch and save to S3
/// @param completeFileLocation
/// @param sravzID
void FundamentalsController::CheckAndFetchData(
    const std::string &sravzID, std::string &completeFileLocation) {
    if (!s3Client_->objectExists(
            FundamentalsController::FUNDAMENTALS_BUCKET_NAME,
            completeFileLocation)) {
        FetchDataAndSaveToS3(sravzID, completeFileLocation);
    }
}

void FundamentalsController::FetchDataAndSaveToS3(
    const std::string &sravzID, std::string &completeFileLocation) {
    const std::string code =
        sravz::utils::ConvertMutualFundSravzIDToCode(sravzID);
    // "/api/eod/{0}?api_token={1}&order=d&fmt=json";
    const std::string target = "/api/eod/" + code + "?api_token=" +
        FundamentalsController::dataProvider_->EODHISTORICALDATA_API_KEY2 +
        "&order=d&fmt=json";
    std::promise<std::string> promise;
    std::future<std::string> future = promise.get_future();
    dataProvider_->httpGet(target, promise);
    // Blocking call
    std::string response = future.get();
    // TODO: PutObjectGZ does not work with Contabo. Check
    s3Client_->putObject(FundamentalsController::FUNDAMENTALS_BUCKET_NAME,
                         completeFileLocation, response);
}

sravz::models::PreSignedUrlModel &FundamentalsController::validate_request(
    sravz::models::PreSignedUrlModel &preSignedUrlModel) { // override {
    preSignedUrlModel.set_param("sravzid", [&preSignedUrlModel](const std::string &v) mutable {
        preSignedUrlModel.set_sravzid(v);
    });
    return preSignedUrlModel;
}
} // namespace sravz::controllers::fundamentals
