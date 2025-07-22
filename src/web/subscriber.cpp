//
// SRAVZ LLC
//

#include "subscriber.hpp"

#include <sravz.hpp>

#include "symbol.hpp"
#include "utils.hpp"

subscriber::subscriber(net::io_context &ioc_subscriber,
                       boost::shared_ptr<shared_state> const &state)
    : ioc_subscriber_(ioc_subscriber),
      redis_ptr_(boost::make_shared<RedisInstance>(
          RedisInstance(sravz::utils::GetRedisConnectionOptions()))),
      state_(state) {}

void subscriber::subscribe() {
    boost::asio::post(net::make_strand(ioc_subscriber_), [&]() {
        // Create a Subscriber.
        // std::cout << "Before calling subscriber"  << std::endl;
        auto topics_subscribed_count = 0;
        auto sub = redis_ptr_->subscriber();
        // for(auto const& symbol_ : state_->symbols_)
        // {
        //     std::cout << "Subscribing to symbol: " << symbol_.first <<
        //     std::endl; sub.subscribe(symbol_.first);
        // }

        // Set callback functions.
        sub.on_message([&](std::string channel, std::string msg) {
            try {
                // if (sessions_.size()) {
                // send(msg);
                // std::cout << msg << std::endl;
                state_->spsc_queue_.push(std::make_pair(channel, msg));
                // }
            } catch (const sw::redis::Error &err) {
                std::cout << "could not send message" << err.what()
                          << std::endl;
            }
        });
        // Consume messages in a loop.
        // std::cout << "Before while loop";
        std::pair<parser::MsgType, std::string> msg_pair;
        while (true) {
            if (topics_subscribed_count) {
                try {
                    sub.consume();
                } catch (const sw::redis::Error &err) {
                    // std::cout << "Cannot read from redis" << err.what() <<
                    // std::endl;
                    // TODO: Handle subscriber failure and recreate subscribe as
                    // per redis++ recommendation
                }
            }
            if (state_->spsc_queue_subscriber_.pop(msg_pair)) {
                auto type = std::get<0>(msg_pair);
                auto topic = std::get<1>(msg_pair);
                // std::cout << "Message in subscribe :"  <<  " " << topic <<
                // std::endl;
                switch (type) {
                    case parser::MsgType::SUBSCRIBE: {
                        std::cout << "Subscribing to topic :" << topic
                                  << std::endl;
                        sub.subscribe(topic);
                        topics_subscribed_count++;
                        break;
                    }
                    case parser::MsgType::UNSUBSCRIBE: {
                        std::cout << "Unsubscribing to topic :" << topic
                                  << std::endl;
                        sub.unsubscribe(topic);
                        topics_subscribed_count--;
                        break;
                    }
                    default:
                        std::cout << "Unsupported message type" << std::endl;
                        break;
                }
            }
        }
    });
}
