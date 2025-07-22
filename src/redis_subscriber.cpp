/**
 * Sravz LLC
 **/
#include <boost/algorithm/string.hpp>
#include <boost/beast.hpp>
#include <boost/foreach.hpp>
#include <boost/json.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/log/trivial.hpp>
#include <boost/thread.hpp>
#include <redis_subscriber.hpp>

namespace beast = boost::beast; // from <boost/beast.hpp>

void RedisSubscriber::subscribe() {
    // Create a Subscriber.
    auto sub = redis_consumer_.subscriber();

    // Set callback functions.
    sub.on_message([&](std::string channel, std::string msg) {
        spsc_queue_parse_.push(msg);
    });

    // Subscribe to channels and patterns.
    std::vector<std::string> topics;
    boost::algorithm::split(topics, topics_, boost::is_any_of(","));
    BOOST_FOREACH (std::string topic, topics) {
        sub.subscribe(topic);
    }

    // Consume messages in a loop.
    while (true) {
        try {
            sub.consume();
        } catch (const sw::redis::Error &err) {
            BOOST_LOG_TRIVIAL(info) << err.what();
        }
    }
}

void RedisSubscriber::parse() {
    std::string msg;
    boost::json::parse_options opt{};
    opt.allow_comments = true;
    opt.allow_trailing_commas = true;
    long last_timestamp = 0;
    while (true) {
        get_message_with_backoff_(spsc_queue_parse_, msg);
        try {
            boost::json::value val = boost::json::parse(msg, {}, opt);
            auto obj = val.as_object();
            auto cur_last_timestamp = boost::json::value_to<long>(obj.at("t"));
            if (last_timestamp > cur_last_timestamp) {
                continue;
            }
            last_timestamp = cur_last_timestamp;
            BOOST_LOG_TRIVIAL(info)
                << "Parse: " << obj.at("s") << " " << obj.at("p") << " "
                << obj.at("t")
                << " thread ID: " << boost::this_thread::get_id();
            spsc_queue_publish_.push(obj);
        } catch (std::exception const &ex) {
            BOOST_LOG_TRIVIAL(info) << "Cannot parse datapoint: " << ex.what();
        }
    }
}

void RedisSubscriber::publish() {
    boost::json::object obj;
    int count = 0;
    std::vector<std::string> result;
    result.push_back("TS.MADD");
    while (true) {
        get_message_with_backoff_(spsc_queue_publish_, obj);
        BOOST_LOG_TRIVIAL(info)
            << "Publish: " << obj.at("s") << " " << obj.at("p") << " "
            << obj.at("t") << " thread ID: " << boost::this_thread::get_id();
        try {
            result.push_back(boost::json::value_to<std::string>(obj.at("s")));
            result.push_back(boost::lexical_cast<std::string>(
                boost::json::value_to<long>(obj.at("t"))));
            result.push_back(boost::json::value_to<std::string>(obj.at("p")));
            if (count >= REDIS_BATCH_SIZE) {
                redis_publisher_.command(result.begin(), result.end());
                count = 0;
                result.resize(1);
            } else {
                count++;
            }
        } catch (std::exception const &ex) {
            BOOST_LOG_TRIVIAL(info) << "Cannot push datapoint: " << ex.what();
        }
    }
}

void RedisSubscriber::run() {
    boost::asio::post(io_->get_executor(),
                      beast::bind_front_handler(&RedisSubscriber::subscribe,
                                                shared_from_this()));
    boost::asio::post(
        io_->get_executor(),
        beast::bind_front_handler(&RedisSubscriber::parse, shared_from_this()));
    boost::asio::post(io_->get_executor(),
                      beast::bind_front_handler(&RedisSubscriber::publish,
                                                shared_from_this()));
}

template <typename T1, typename T2>
void RedisSubscriber::get_message_with_backoff_(
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
