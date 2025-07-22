#pragma once
#include <sw/redis++/errors.h>
#include <sw/redis++/redis++.h>

#include <algorithm>
#include <locale>
#include <sstream>
#include <stdexcept> // For std::invalid_argument
#include <string>

namespace sravz::utils {
auto Getenv(const char *name, std::string &env) -> bool;
auto GetRedisConnectionOptions() -> sw::redis::ConnectionOptions;
std::string CompressData(const std::string &data);
std::vector<std::string> GetStringTokens(const std::string &data);
std::string ToUpper(const std::string &data);

// Financial Utils
std::string ConvertMutualFundSravzIDToCode(const std::string &data);

// Exceptions
class InvalidInputException : public std::invalid_argument {
public:
    InvalidInputException(const std::string &message)
        : std::invalid_argument(message) {}
};

} // namespace sravz::utils
