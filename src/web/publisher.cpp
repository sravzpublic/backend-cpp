//
// SRAVZ LLC
//

#include "publisher.hpp"

publisher::publisher(net::io_context &ioc_publisher,
                     boost::shared_ptr<shared_state> const &state)
    : ioc_publisher_(ioc_publisher), state_(state) {}

void publisher::publish() {
    boost::asio::post(net::make_strand(ioc_publisher_), [&]() {
        std::pair<std::string, std::string> msg_pair;
        while (true) {
            sravz::get_message_with_backoff(state_->spsc_queue_, msg_pair);
            auto topic = std::get<0>(msg_pair);
            auto message = std::get<1>(msg_pair);
            // Put the message in a shared pointer so we can re-use it for each
            // client
            auto const ss =
                boost::make_shared<std::string const>(std::move(message));

            // Make a local list of all the weak pointers representing
            // the sessions, so we can do the actual sending without
            // holding the mutex:
            std::vector<boost::weak_ptr<websocket_session>> v;
            {
                std::lock_guard<std::mutex> lock_symbol(
                    state_->symbols_[topic]->mutex_);
                v.reserve(state_->symbols_[topic]->sessions_.size());
                for (auto p : state_->symbols_[topic]->sessions_) {
                    v.emplace_back(p->weak_from_this());
                }
            }

            // For each session in our local list, try to acquire a strong
            // pointer. If successful, then send the message on that session.
            for (auto const &wp : v) {
                if (auto sp = wp.lock()) {
                    sp->send(ss);
                }
            }
        }
    });
}
