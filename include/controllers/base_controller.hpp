#pragma once
#include <boost/beast.hpp>
#include <map>
#include <optional>
#include <string>

#include "base_model.hpp"

namespace sravz::controller {
class BaseController {
public:
    virtual ~BaseController() = default;
    virtual sravz::models::BaseModel &get(sravz::models::BaseModel &baseModel) {
        return baseModel;
    };
    virtual sravz::models::BaseModel &
    validate_request(sravz::models::BaseModel &baseModel) {
        return baseModel;
    };

protected:
    BaseController() = default;
};
} // namespace sravz::controller