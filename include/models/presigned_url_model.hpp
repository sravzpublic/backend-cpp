#pragma once
#include <boost/json.hpp>
namespace json = boost::json;

#include "base_model.hpp"

namespace sravz::models {
class PreSignedUrlModel : public BaseModel {
public:
    PreSignedUrlModel() = default;
    PreSignedUrlModel(std::map<std::string, std::string> params_map)
        : BaseModel(params_map){};
    virtual ~PreSignedUrlModel() = default;

private:
    std::string id;
    std::string bucket;
    std::string key;
    std::string url;
    std::string expires;
    std::string sravzid;

public:
    const std::string &get_id() const { return id; }
    std::string &get_mutable_id() { return id; }
    void set_id(const std::string &value) { this->id = value; }

    const std::string &get_bucket() const { return bucket; }
    std::string &get_mutable_bucket() { return bucket; }
    void set_bucket(const std::string &value) { this->bucket = value; }

    const std::string &get_key() const { return key; }
    std::string &get_mutable_key() { return key; }
    void set_key(const std::string &value) { this->key = value; }

    const std::string &get_url() const { return url; }
    std::string &get_mutable_url() { return url; }
    void set_url(const std::string &value) { this->url = value; }

    const std::string &get_expires() const { return expires; }
    std::string &get_mutable_expires() { return expires; }
    void set_expires(const std::string &value) { this->expires = value; }

    const std::string &get_sravzid() const { return sravzid; }
    std::string &get_mutable_sravzid() { return sravzid; }
    void set_sravzid(const std::string &value) { this->sravzid = value; }

    std::string to_json() override {
        json::object obj;
        obj["Bucket"] = bucket;
        obj["Key"] = key;
        obj["URL"] = url;
        obj["Expires"] = expires;
        obj["id"] = id;
        return json::serialize(obj);
    }
};
} // namespace sravz::models