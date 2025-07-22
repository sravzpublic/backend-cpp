// Sravz LLC
// Ref:
// https://github.com/mongodb/mongo-cxx-driver/blob/142c5e39766cb0f5515d55f37b95acb20ca790e2/examples/mongocxx/instance_management.cpp
#pragma once

#include <boost/json.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>
#include <sstream>
#include <vector>

// Redefine assert after including headers. Release builds may undefine the
// assert macro and result in -Wunused-variable warnings.
#if defined(NDEBUG) || !defined(assert)
#undef assert
#define assert(stmt)                                                           \
    do {                                                                       \
        if (!(stmt)) {                                                         \
            std::cerr << "Assert on line " << __LINE__ << " failed: " << #stmt \
                      << std::endl;                                            \
            abort();                                                           \
        }                                                                      \
    } while (0)
#endif

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

namespace sravz {
namespace services {
    namespace mongo {
        class noop_logger : public mongocxx::logger {
        public:
            virtual void operator()(mongocxx::log_level,
                                    bsoncxx::stdx::string_view,
                                    bsoncxx::stdx::string_view) noexcept {}
        };

        class MongoClient {
        public:
            MongoClient(std::string url)
                : url_(url)

            {
                // This should be done only once.
                auto instance = bsoncxx::stdx::make_unique<mongocxx::instance>(
                    bsoncxx::stdx::make_unique<noop_logger>());
                configure(std::move(instance),
                          bsoncxx::stdx::make_unique<mongocxx::pool>(
                              std::move(mongocxx::uri{url_})));
            }

            void configure(std::unique_ptr<mongocxx::instance> instance,
                           std::unique_ptr<mongocxx::pool> pool) {
                _instance = std::move(instance);
                _pool = std::move(pool);
            }

            using connection = mongocxx::pool::entry;

            connection get_connection() {
                // Thread blocks until a connection is available
                return _pool->acquire();
            }

            bsoncxx::stdx::optional<connection> try_get_connection() {
                // Retruns immediately if the connection is not available
                return _pool->try_acquire();
            }

            std::string insert(std::string database, std::string collection,
                               std::string documment_json);
            std::string insert_many(const std::string &database,
                                    const std::string &collection,
                                    const std::string &documents_json);
            std::string upsert(const std::string &database,
                               const std::string &collection_name,
                               const std::string &search_json,
                               const std::string &update_json,
                               const bool &update_one);
            std::string find_one(std::string database, std::string collection,
                                 std::string documment_json);
            std::string find_all(std::string database, std::string collection,
                                 std::string documment_json);
            std::string _delete(const std::string &database,
                                const std::string &collection_name,
                                const std::string &document_json,
                                const bool &delete_one);

        private:
            std::string url_;
            std::unique_ptr<mongocxx::instance> _instance = nullptr;
            std::unique_ptr<mongocxx::pool> _pool = nullptr;
            boost::json::parse_options opt_{};
        };
    } // namespace mongo
} // namespace services
} // namespace sravz
