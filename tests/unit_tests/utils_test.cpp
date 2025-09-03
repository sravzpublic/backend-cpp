#include "utils.hpp"

#include <gtest/gtest.h>

TEST(utils, ConvertMutualFundSravzIDToCode) {
    const std::string code =
        sravz::utils::ConvertMutualFundSravzIDToCode("fund_us_test");
    ASSERT_EQ(code, "TEST.US");
}

TEST(utils, GetStringTokens) {
    std::vector<std::string> expected = {"fund", "us", "test"};
    auto tokens = sravz::utils::GetStringTokens("fund_us_test");
    ASSERT_EQ(tokens, expected);
    ASSERT_EQ(tokens.size(), 3);
}

TEST(utils, ToUpper) {
    auto upperCaseSravzID = sravz::utils::ToUpper("fund_us_test");
    ASSERT_EQ(upperCaseSravzID, "FUND_US_TEST");
}

TEST(utils, InvalidSravzID) {
    ASSERT_THROW(sravz::utils::ConvertMutualFundSravzIDToCode("us_test"),
                 sravz::utils::InvalidInputException);
}
