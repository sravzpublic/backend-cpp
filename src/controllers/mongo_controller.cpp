#include <boost/shared_ptr.hpp>
#include <mongo_controller.hpp>

namespace sravz {
namespace controllers {
    namespace mongo {
        std::string MongoController::get(std::string collection,
                                         std::string document_json) {
            return mongoClient_->find_one("sravz", collection, document_json);
        }

        std::string MongoController::post(const std::string &collection,
                                          const std::string &document_json) {
            return mongoClient_->insert_many("sravz", collection,
                                             document_json);
        }

        std::string MongoController::put(const std::string &collection,
                                         const std::string &search_json,
                                         const std::string &update_json,
                                         const bool &update_one) {
            return mongoClient_->upsert("sravz", collection, search_json,
                                        update_json, update_one);
        }

        std::string MongoController::_delete(const std::string &collection,
                                             const std::string &search_json,
                                             const bool &delete_one) {
            return mongoClient_->_delete("sravz", collection, search_json,
                                         delete_one);
        }
    } // namespace mongo
} // namespace controllers
} // namespace sravz