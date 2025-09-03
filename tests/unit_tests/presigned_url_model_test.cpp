#include "presigned_url_model.hpp"

#include <gtest/gtest.h>

TEST(presigned_url_model, to_json) {
    std::map<std::string, std::string> params_map = {
        {"sravzid", "commonstock_us_arkvx.json1"}};
    auto model = sravz::models::PreSignedUrlModel(params_map);
    model.set_bucket("test_bucket");
    model.set_key("test_key");
    model.set_url("test_url");
    model.set_expires("test_expires");
    model.set_id("commonstock_us_arkvx.json1");

    std::string jsonStr = model.to_json();

    std::string expectedJson =
        R"({"Bucket":"test_bucket","Key":"test_key","URL":"test_url","Expires":"test_expires","id":"commonstock_us_arkvx.json1"})";
    ASSERT_EQ(jsonStr, expectedJson);
}
