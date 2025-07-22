#pragma once
#include <boost/json.hpp>
namespace json = boost::json;

#include "base_model.hpp"

namespace sravz::models {
class DuckDBModel : public BaseModel {
public:
    DuckDBModel() = default;
    DuckDBModel(std::map<std::string, std::string> params_map)
        : BaseModel(params_map) {};
    virtual ~DuckDBModel() = default;

private:
    std::string bucket;
    std::string key;
    std::string where_clause;
    boost::json::array db_query_result;

public:
    const std::string &get_bucket() const { return bucket; }
    std::string &get_mutable_bucket() { return bucket; }
    void set_bucket(const std::string &value) { this->bucket = value; }

    const std::string &get_key() const { return key; }
    std::string &get_mutable_key() { return key; }
    void set_key(const std::string &value) { this->key = value; }

    const std::string &get_where_clause() const { return where_clause; }
    std::string &get_mutable_where_clause() { return where_clause; }
    void set_where_clause(const std::string &value) {
        this->where_clause = value;
    }

    const boost::json::array &get_db_query_result() const {
        return db_query_result;
    }
    void set_db_query_result(const boost::json::array &value) {
        db_query_result = value;
    }
    void set_db_query_result_from_string(const boost::json::array &value) {
        db_query_result = value;
    }

    std::string to_json() override {
        json::object obj;
        obj["Bucket"] = bucket;
        obj["DbQueryResult"] = db_query_result;
        obj["Key"] = key;
        obj["WhereClause"] = where_clause;
        return json::serialize(obj);
    }
};
} // namespace sravz::models