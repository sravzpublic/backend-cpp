#pragma once
#include <boost/lockfree/queue.hpp>
#include <string>

namespace sravz {

const int EXPONENTIAL_BACKOFF_LIMIT = 5;

struct config {
    int id;
    std::string topics;
};

template <typename T1, typename T2>
void get_message_with_backoff(
    boost::lockfree::spsc_queue<T1, boost::lockfree::capacity<1024>> &queue,
    T2 &msg) {
    int backoff = 0;
    while (true) {
        if (queue.pop(msg)) {
            return;
        } else {
            sleep(1 << backoff++);
            if (backoff > EXPONENTIAL_BACKOFF_LIMIT) {
                backoff = 0;
            }
        }
    }
}
} // namespace sravz

// void tag_invoke(boost::json::value_from_tag, boost::json::value& jv, config
// const& c)
// {
//     jv =
//     {
//         { "id" , c.id },
//         { "topics", c.topics }
//     };
// };

// config tag_invoke(boost::json::value_to_tag< config >, boost::json::value
// const& jv )
// {
//     object const& obj = jv.as_object();
//     return config {
//         value_to<int>( obj.at( "id" ) ),
//         value_to<std::string>( obj.at( "topics" ) )
//     };
// };
