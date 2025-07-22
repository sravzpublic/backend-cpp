#pragma once
#include <sw/redis++/errors.h>
#include <sw/redis++/redis++.h>

#include <string>

using RedisInstance = sw::redis::Redis;

namespace sravz::services::redis {

auto getRedisConnectionOptions() -> sw::redis::ConnectionOptions;

class RedisClient {
public:
    RedisClient() : redisClient_(getRedisConnectionOptions()) {}
    bool set(const std::string &key, const std::string &value);
    bool set_with_expiry(const std::string &key, const std::string &value, const std::chrono::seconds expiry);
    std::optional<std::string> get(const std::string &key);    
    ~RedisClient();

private:
    std::string url_;
    sw::redis::Redis redisClient_;
};
} // namespace sravz::services::redis
