// Sravz LLC
#include <mongo_service.hpp>

namespace sravz {
namespace services {
    namespace mongo {
        std::string MongoClient::insert(std::string database,
                                        std::string collection_name,
                                        std::string document_json) {
            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            auto result = collection.insert_one(
                std::move(bsoncxx::v_noabi::from_json(document_json)));
            if (result)
                return result->inserted_id().get_oid().value.to_string();
            return "error";
        }

        std::string MongoClient::insert_many(const std::string &database,
                                             const std::string &collection_name,
                                             const std::string &document_json) {
            boost::json::value jsonValue =
                boost::json::parse(document_json, {}, opt_);
            if (jsonValue.kind() != boost::json::kind::array)
                return "Not an array";

            auto const &arr = jsonValue.get_array();
            std::vector<bsoncxx::document::value> documents;

            if (arr.empty())
                return "Empty array";

            auto it = arr.begin();
            for (;;) {
                // std::cout << boost::json::value_from( *it ) << std::endl;
                std::ostringstream ss;
                ss << boost::json::value_from(*it);
                documents.emplace_back(bsoncxx::v_noabi::from_json(ss.str()));
                if (++it == arr.end())
                    break;
            }

            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            auto insert_many_result = collection.insert_many(documents);
            if (insert_many_result) {
                std::map<size_t, bsoncxx::document::element> inserted_ids{};
                inserted_ids = insert_many_result->inserted_ids();
                auto result = bsoncxx::builder::basic::document{};
                bsoncxx::builder::basic::document inserted_ids_builder;
                for (auto &&id_pair : inserted_ids) {
                    inserted_ids_builder.append(
                        kvp(std::to_string(id_pair.first),
                            id_pair.second.get_value()));
                }
                result.append(
                    kvp("result",
                        make_document(
                            kvp("insertedIds", inserted_ids_builder.extract()),
                            kvp("insertedCount",
                                insert_many_result->inserted_count()))));
                return bsoncxx::v_noabi::to_json(result.extract());
            }
            return "error";
        }

        std::string MongoClient::upsert(const std::string &database,
                                        const std::string &collection_name,
                                        const std::string &search_json,
                                        const std::string &update_json,
                                        const bool &update_one) {
            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            auto bulk = collection.create_bulk_write();
            if (update_one) {
                mongocxx::model::update_one upsert_op{
                    bsoncxx::v_noabi::from_json(search_json),
                    make_document(
                        kvp("$set", bsoncxx::v_noabi::from_json(update_json)))};
                upsert_op.upsert(true);
                bulk.append(upsert_op);
            } else {
                mongocxx::model::update_many upsert_op{
                    bsoncxx::v_noabi::from_json(search_json),
                    make_document(
                        kvp("$set", bsoncxx::v_noabi::from_json(update_json)))};
                upsert_op.upsert(true);
                bulk.append(upsert_op);
            }
            auto upserted_result = bulk.execute();
            if (upserted_result) {
                std::map<size_t, bsoncxx::document::element> upserted_ids{};
                upserted_ids = upserted_result->upserted_ids();
                auto result = bsoncxx::builder::basic::document{};
                bsoncxx::builder::basic::document upserted_ids_builder;
                for (auto &&id_pair : upserted_ids) {
                    // std::cout << "Upserted document ID: " <<
                    // id_pair.get_oid().value.to_string() << std::endl;
                    upserted_ids_builder.append(
                        kvp(std::to_string(id_pair.first),
                            id_pair.second.get_value()));
                }
                result.append(
                    kvp("result",
                        make_document(
                            kvp("upsertedIds", upserted_ids_builder.extract()),
                            kvp("insertedCount",
                                upserted_result->upserted_count()))));
                return bsoncxx::v_noabi::to_json(result.extract());
            }
            return "{}";
        }

        std::string MongoClient::find_one(std::string database,
                                          std::string collection_name,
                                          std::string document_json) {
            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            auto find_one_filtered_result =
                collection.find_one(bsoncxx::v_noabi::from_json(document_json));
            if (find_one_filtered_result) {
                return bsoncxx::to_json(*find_one_filtered_result);
            }
            return "{}";
        }

        std::string MongoClient::find_all(std::string database,
                                          std::string collection_name,
                                          std::string document_json) {
            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            auto cursor_filtered =
                collection.find(bsoncxx::v_noabi::from_json(document_json));
            std::ostringstream ss;
            ss << "[";
            bool found = false;
            for (auto doc : cursor_filtered) {
                // std::cout << "Find all - document: " << bsoncxx::to_json(doc,
                // bsoncxx::ExtendedJsonMode::k_relaxed) << std::endl;
                ss << bsoncxx::to_json(doc,
                                       bsoncxx::ExtendedJsonMode::k_relaxed)
                   << ",";
            }
            if (found)
                ss.seekp(-1, ss.cur);
            ss << "]";
            return ss.str();
        }

        std::string MongoClient::_delete(const std::string &database,
                                         const std::string &collection_name,
                                         const std::string &document_json,
                                         const bool &delete_one) {
            auto client = get_connection();
            auto collection = (*client)[database][collection_name];
            // std::map<size_t, bsoncxx::document::element> deleted_ids{};
            std::int32_t deleted_count = 0;
            std::optional<mongocxx::result::delete_result> delete_result;
            if (delete_one) {
                delete_result = collection.delete_one(
                    bsoncxx::v_noabi::from_json(document_json));
                // if (delete_result) {
                //  deleted_ids = delete_result->deleted_ids();
                //    deleted_count = delete_result->deleted_count();
                // }
            } else {
                delete_result = collection.delete_many(
                    bsoncxx::v_noabi::from_json(document_json));
                // if (delete_result) {
                //  deleted_ids = delete_result->deleted_ids();
                //    deleted_count = delete_result->deleted_count();
                // }
            }

            if (delete_result)
                deleted_count = delete_result->deleted_count();
            // deleted_ids.size()) {
            auto result = bsoncxx::builder::basic::document{};
            // bsoncxx::builder::basic::document deleted_ids_builder;
            // for (auto&& id_pair : deleted_ids) {
            //     deleted_ids_builder.append(kvp(std::to_string(id_pair.first),
            //     id_pair.second.get_value()));
            // }
            result.append(kvp(
                "result", make_document(kvp("deletedCount", deleted_count))));
            return bsoncxx::v_noabi::to_json(result.extract());
        }
    } // namespace mongo
} // namespace services
} // namespace sravz