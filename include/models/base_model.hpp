#pragma once
#include <boost/beast.hpp>
#include <map>
#include <string>

namespace sravz::models {
class BaseModel {
public:
    virtual ~BaseModel() = default;
    virtual std::string to_json() = 0;
    BaseModel() = default;
    boost::beast::http::status status;
    std::string status_message;
    std::map<std::string, std::string> params_map_;
    std::vector<std::string> missing_params;    
    BaseModel(std::map<std::string, std::string> params_map)
        : params_map_(params_map) {
            status = boost::beast::http::status::ok;
        };     
    template<typename Setter>
    void set_param(const std::string& param_name , Setter setter) {
        auto it = params_map_.find(param_name);
        if (it != params_map_.end()) {
            setter(it->second);
        } else {
            missing_params.push_back(param_name);
        }
    }      
    void set_missing_params_status() {
        if (!missing_params.empty()) {
            status = boost::beast::http::status::bad_request;
            status_message = "Parameter(s) missing: ";
            for (size_t i = 0; i < missing_params.size(); ++i) {
                status_message += missing_params[i];
                if (i != missing_params.size() - 1) {
                    status_message += ", ";
                }
            }
        }
    }

};
} // namespace sravz::models