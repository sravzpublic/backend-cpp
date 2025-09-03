#include <gtest/gtest.h>

#include <redis_service.hpp>

TEST(redis_service, integration) {
    sravz::services::redis::RedisClient redisClient;
    ASSERT_TRUE(redisClient.set("Hello", "World"));
    std::optional<std::string> valueForHelloKey = redisClient.get("Hello");
    ASSERT_TRUE(valueForHelloKey.has_value());
    ASSERT_EQ(valueForHelloKey.value(), "World");
}