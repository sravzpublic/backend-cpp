// Sravz LLC
#include <redis_service.hpp>
#include <utils.hpp>

using namespace sw::redis;

namespace sravz::services::redis {
// Returns redis connection options to be used in redis connection
// creation
sw::redis::ConnectionOptions getRedisConnectionOptions() {
    std::string REDIS_HOST;
    std::string REDIS_PORT;
    std::string REDIS_PASSWORD;
    sw::redis::ConnectionOptions opts;
    if (sravz::utils::Getenv("REDIS_HOST", REDIS_HOST))
        opts.host = REDIS_HOST;
    if (sravz::utils::Getenv("REDIS_PORT", REDIS_PORT))
        opts.port = std::stoi(REDIS_PORT);
    if (sravz::utils::Getenv("REDIS_PASSWORD", REDIS_PASSWORD))
        opts.password = REDIS_PASSWORD;
    // TODO: Temp fix
    opts.host = "redis";
    opts.port = 6379;
    opts.socket_timeout = std::chrono::milliseconds(200);
    return opts;
}

bool RedisClient::set(const std::string &key, const std::string &value) {
    return redisClient_.set(key, value);
}

bool RedisClient::set_with_expiry(const std::string &key,
                                  const std::string &value,
                                  const std::chrono::seconds expiry) {
    return redisClient_.set(key, value, expiry);
}

std::optional<std::string> RedisClient::get(const std::string &key) {
    return redisClient_.get(key);
}

RedisClient::~RedisClient() {
    // TOOD: Check if any cleanup is needed
    // Aws::ShutdownAPI(options_);
}
} // namespace sravz::services::redis
