#include "utils.hpp"

#include <sw/redis++/connection.h>
#include <zlib.h>

#include <boost/log/trivial.hpp>
#include <cstdlib>
#include <string>

// Gets env variable
namespace sravz::utils {
auto Getenv(const char *name, std::string &env) -> bool {
    const char *ret = std::getenv(name);
    if (ret != nullptr) {
        env = std::string(ret);
    } else {
        BOOST_LOG_TRIVIAL(error) << "Env variable: " << name << " not set!!!";
    }
    return !(ret == nullptr);
}

auto GetRedisConnectionOptions() -> sw::redis::ConnectionOptions {
    std::string redis_host;
    std::string redis_port;
    std::string redis_password;
    sw::redis::ConnectionOptions opts;
    if (Getenv("REDIS_HOST", redis_host)) {
        opts.host = redis_host;
    }
    if (Getenv("REDIS_PORT", redis_port)) {
        opts.port = std::stoi(redis_port);
    }
    if (Getenv("REDIS_PASSWORD", redis_password)) {
        opts.password = redis_password;
    }
    opts.socket_timeout = std::chrono::milliseconds(200);
    return opts;
}

// Function to compress data using zlib (gzip)
std::string CompressData(const std::string &data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED, 15 | 16, 8,
                     Z_DEFAULT_STRATEGY) != Z_OK) {
        throw(std::runtime_error("deflateInit2 failed while compressing."));
    }

    zs.next_in = (Bytef *)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // Retrieve the compressed bytes
    do {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw(std::runtime_error("Exception during zlib compression: (" +
                                 std::to_string(ret) + ") " + zs.msg));
    }

    return outstring;
}

std::vector<std::string> GetStringTokens(const std::string &data) {
    std::stringstream ss(data);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(ss, token, '_')) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string ToUpper(const std::string &data) {
    std::string result = data;
    for (char &c : result) {
        c = std::toupper(c);
    }
    return result;
}

std::string ConvertMutualFundSravzIDToCode(const std::string &sravzID) {
    // Convert fund_us_aaaxc to AAAXC.US
    auto sravzIDTokens = GetStringTokens(sravzID);

    if (sravzIDTokens.size() != 3) {
        throw InvalidInputException("Invalid SravzID: " + sravzID +
                                    ". Cannot generate Code");
    }
    auto assetCode = ToUpper(sravzIDTokens[2] + "." + sravzIDTokens[1]);
    return assetCode;
}

} // namespace sravz::utils
