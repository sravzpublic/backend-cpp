/**
 * Sravz LLC
 **/
#include <mongo_service.hpp>

using namespace sravz::services::mongo;
using namespace std;

int main() {
    auto mongoClient =
        MongoClient(std::move("mongodb://sravz:sravz@mongo:27017/sravz"));

    // Insert a document
    cout << "Inserted document ID: "
         << mongoClient.insert(
                "sravz", "books",
                "{\"name\":\"book1\",\"description\":\"book1description\","
                "\"author\":\"book1author\",\"genre\":\"book1genre\"}")
         << std::endl;

    // Insert multiple documents
    cout << "Insert multiple: "
         << mongoClient.insert_many("sravz", "books",
                                    R""""(
                                    [
                                        {"name":"book2","description":"book2description","author":"book2author","genre": "book2genre"},
                                        {"name":"book3","description":"book3description","author":"book3author","genre":"book3genre"},
                                        {"name":"book3","description":"book3description","author":"book3author","genre":"book3genre"},
                                        {"name":"book3","description":"book3description","author":"book3author","genre":"book3genre"},
                                        {"name":"book4","description":"book4description","author":"book4author","genre": "book4genre"},
                                        {"name":"book4","description":"book4description","author":"book4author","genre": "book4genre"},
                                        {"name":"book4","description":"book4description","author":"book4author","genre": "book4genre"},
                                        {"name":"book4","description":"book4description","author":"book4author","genre": "book4genre"}
                                    ]
                                    )"""")
         << std::endl;

    // Find a document matching a filter
    cout << "Find: "
         << mongoClient.find_one("sravz", "books", "{\"name\":\"book2\"}")
         << std::endl;

    // Find all documents matching a filter
    cout << "Find all:"
         << mongoClient.find_all("sravz", "books", "{\"name\":\"book2\"}")
         << std::endl;

    // Upsert one document
    cout << "Upsert one:"
         << mongoClient.upsert(
                "sravz", "books", "{\"name\":\"book3\"}",
                "{\"description\":\"book3description-upserted\"}", true)
         << std::endl;

    // Upsert many document
    cout << "Upsert many:"
         << mongoClient.upsert(
                "sravz", "books", "{\"name\":\"book3\"}",
                "{\"description\":\"book3description-upserted\"}", false)
         << std::endl;

    // Find all after upsert
    // TODO: Fix upserted IDs are not returned but the documents are updated
    cout << "Find all book3 after upsert:"
         << mongoClient.find_all("sravz", "books", "{\"name\":\"book3\"}")
         << std::endl;

    // Delete one document
    cout << "Delete one:"
         << mongoClient._delete("sravz", "books", "{\"name\":\"book4\"}", true)
         << std::endl;

    // Delete all document
    cout << "Delete all:"
         << mongoClient._delete("sravz", "books", "{\"name\":\"book4\"}", false)
         << std::endl;

    // Find all after delete
    cout << "Find all book4 after delete:"
         << mongoClient.find_all("sravz", "books", "{\"name\":\"book4\"}")
         << std::endl;
}