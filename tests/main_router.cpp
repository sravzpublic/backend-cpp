/**
 * Sravz LLC
 **/
#include <boost/beast/http.hpp>
#include <iostream>
#include <router.hpp>

using namespace std;
namespace http = boost::beast::http;

void print_result(
    const std::pair<boost::beast::http::status, std::string> &result) {
    std::cout << " HTTP Code: " << result.first
              << " Response: " << result.second << std::endl;
}

int main() {
    auto router = sravz::router();
    // Invalid route
    print_result(router.route(http::verb::get, "https://localhost/",
                              "{\"name\":\"book2\"}"));
    // Invalid service
    print_result(router.route(http::verb::get, "https://localhost/test",
                              "{\"name\":\"book2\"}"));
    // Correct service mdb but no collection name
    print_result(router.route(http::verb::get, "https://localhost/mdb",
                              "{\"name\":\"book2\"}"));
    // Correct service mdb and correct collection name
    print_result(router.route(http::verb::get,
                              "https://localhost/mdb/books?name=book3", ""));
    // Post multiple documents
    print_result(
        router.route(http::verb::post, "https://localhost/mdb/books", R""""(
                                    [
                                        {"name":"book6","description":"book6description","author":"book6author","genre": "book6genre"},
                                        {"name":"book7","description":"book7description","author":"book7author","genre":"book7genre"}
                                    ]
                                    )""""));
    // Get posted books
    print_result(router.route(http::verb::get,
                              "https://localhost/mdb/books?name=book6", ""));
    print_result(router.route(http::verb::get,
                              "https://localhost/mdb/books?name=book7", ""));
    // Put document
    print_result(
        router.route(http::verb::put, "https://localhost/mdb/books", R""""(
                                        {
                                            "filter":  {"name":"book6"},
                                            "update":  {"description":"book6description-upserted"},
                                            "update_one":  true
                                        }
                                    )""""));
    // Get upserted book
    print_result(router.route(http::verb::get,
                              "https://localhost/mdb/books?name=book6", ""));
    // TODO: Delete book
    print_result(
        router.route(http::verb::delete_, "https://localhost/mdb/book", R""""(
                                        {
                                            "filter":  {"name":"book6"},
                                            "delete_one":  false
                                        }
                                    )""""));
    // Get delete book
    print_result(router.route(http::verb::get,
                              "https://localhost/mdb/books?name=book6", ""));
}