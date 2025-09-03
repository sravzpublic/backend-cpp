#include <gtest/gtest.h>

#include <cstdlib>
// Include the header file where getConfigFile function is declared
#include "main.hpp"

// Test case when NODE_ENV is set
TEST(main, NodeEnvSet) {
    setenv("NODE_ENV", "test", 1);
    std::string config_file;
    EXPECT_TRUE(getConfigFile(config_file));
    EXPECT_EQ(config_file, "app.test.cfg");
}

// Test case when NODE_ENV is not set
TEST(main, NodeEnvNotSet) {
    unsetenv("NODE_ENV");
    std::string config_file;
    EXPECT_FALSE(getConfigFile(config_file));
}