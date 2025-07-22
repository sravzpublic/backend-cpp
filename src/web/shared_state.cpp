//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "shared_state.hpp"

#include "symbol.hpp"
#include "websocket_session.hpp"

shared_state::shared_state(std::string doc_root, std::string topics,
                           net::io_context &ioc_background_worker,
                           boost::program_options::variables_map vm,
                           boost::program_options::variables_map vm_env)
    : doc_root_(std::move(doc_root)),
      topics_(topics),
      ioc_background_worker_(ioc_background_worker),
      vm(vm),
      vm_env(vm_env),
      router_(sravz::router(ioc_background_worker)) {
    init();
}

void shared_state::init() {
    // Parse comma separated topics and update symbols
    std::vector<std::string> topics;
    boost::algorithm::split(topics, topics_, boost::is_any_of(","));
    boost::algorithm::split(allowed_origins_,
                            vm["allowed-origins"].as<std::string>(),
                            boost::is_any_of(","));
    for (auto topic : topics) {
        symbols_[topic] = boost::make_shared<symbol>(
            topic, "",
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch())
                .count());
    }
}

void shared_state::join(websocket_session *session) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Inserting into session" << std::endl;
    sessions_.insert(session);
}

void shared_state::websocket_subscribe_to_symbols(
    websocket_session *session, std::vector<std::string> topics) {
    for (auto topic : topics) {
        // Lock shared state mutex
        // Insert into websocket set
        {
            std::lock_guard<std::mutex> lock(mutex_);
            session->topics.insert(topic);
        }
        // Lock symbol mutex
        // Insert into symbol set
        std::lock_guard<std::mutex> lock(symbols_[topic]->mutex_);
        // First websocket to subscribe to the symbol
        if (!symbols_[topic]->sessions_.size()) {
            spsc_queue_subscriber_.push(
                std::make_pair(parser::MsgType::SUBSCRIBE, topic));
        }
        symbols_[topic]->join(session);
        std::cout << "Session size :" << symbols_[topic]->sessions_.size()
                  << std::endl;
    }
}

template <typename T>
void shared_state::websocket_unsubscribe_to_symbols(websocket_session *session,
                                                    T topics) {
    for (auto topic : topics) {
        // Lock shared state mutex
        // Remove from websocket set
        {
            std::lock_guard<std::mutex> lock(mutex_);
            session->topics.erase(topic);
        }
        std::lock_guard<std::mutex> lock(symbols_[topic]->mutex_);
        if (symbols_[topic]->leave(session)) {
            if (!symbols_[topic]->sessions_.size()) {
                spsc_queue_subscriber_.push(
                    std::make_pair(parser::MsgType::UNSUBSCRIBE, topic));
            }
        }
    }
}

void shared_state::leave(websocket_session *session) {
    // Remove the websocket from each symbol subscription
    // websocket_unsubscribe_symbols(session);
    websocket_unsubscribe_to_symbols(session, session->topics);
    // Remove web socket from global session
    // std::cout << "Leaving session" << std::endl;
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
}

void shared_state::parse(std::string msg, websocket_session *session) {
    try {
        auto obj = parser_.parse(msg);
        std::vector<std::string> topics;
        switch (static_cast<parser::MsgType>(
            boost::json::value_to<int>(obj.at("ty")))) {
            case parser::MsgType::SUBSCRIBE: {
                // Parse comma separated topics and update symbols
                boost::algorithm::split(
                    topics, boost::json::value_to<std::string>(obj.at("to")),
                    boost::is_any_of(","));
                websocket_subscribe_to_symbols(session, topics);
                break;
            }
            case parser::MsgType::UNSUBSCRIBE: {
                boost::algorithm::split(
                    topics, boost::json::value_to<std::string>(obj.at("to")),
                    boost::is_any_of(","));
                websocket_unsubscribe_to_symbols(session, topics);
                break;
            }
            default:
                std::cout << "Unsupported message type" << std::endl;
                break;
        }
    } catch (std::exception &e) {
        std::cout << e.what() << "\n";
    }
}
