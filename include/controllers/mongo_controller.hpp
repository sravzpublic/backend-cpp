#pragma once
#include <memory>
#include <mongo_service.hpp>

namespace sravz {
namespace controllers {
    namespace mongo {
        class MongoController {
        public:
            MongoController(std::shared_ptr<sravz::services::mongo::MongoClient>
                                &mongoClient)
                : mongoClient_(mongoClient) {}
            std::string get(std::string collection, std::string documment_json);
            std::string post(const std::string &collection,
                             const std::string &documment_json);
            std::string put(const std::string &collection,
                            const std::string &search_json,
                            const std::string &update_json,
                            const bool &update_one);
            std::string _delete(const std::string &collection,
                                const std::string &search_json,
                                const bool &delete_one);

        private:
            std::shared_ptr<sravz::services::mongo::MongoClient> mongoClient_;
        };
    } // namespace mongo
} // namespace controllers
} // namespace sravz