// hello_duckdb.cpp
#include "duckdb.hpp"

int main() {
    duckdb::DuckDB db(nullptr);
    duckdb::Connection con(db);
    auto result = con.Query("SELECT 'hello duckdb!' as message");
    result->Print();
    return 0;
}