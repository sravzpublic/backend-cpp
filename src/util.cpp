#include <sw/redis++/connection.h>

#include <cstdlib>
#include <iostream>
#include <string>
#include <util.hpp>

// Gets env variable
auto getenv(const char *name, std::string &env) -> bool {
    const char *ret = getenv(name);
    if (ret != nullptr) {
        env = std::string(ret);
    } else {
        std::cout << "Env variable: " << name << " not set!!!";
    }
    return !(ret == nullptr);
}

// Returns redis connection options to be used in redis connection creation
auto getRedisConnectionOptions() -> sw::redis::ConnectionOptions {
    std::string redis_host;
    std::string redis_port;
    std::string redis_password;
    sw::redis::ConnectionOptions opts;
    if (getenv("REDIS_HOST", redis_host)) {
        opts.host = redis_host;
    }
    if (getenv("REDIS_PORT", redis_port)) {
        opts.port = std::stoi(redis_port);
    }
    if (getenv("REDIS_PASSWORD", redis_password)) {
        opts.password = redis_password;
    }
    opts.socket_timeout = std::chrono::milliseconds(200);
    return opts;
}
