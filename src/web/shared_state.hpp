//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#ifndef BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_SHARED_STATE_HPP
#define BOOST_BEAST_EXAMPLE_WEBSOCKET_CHAT_MULTI_SHARED_STATE_HPP

#include <router.hpp>
#include <util.hpp>

#include "parser.hpp"

// Forward declaration
class websocket_session;
class symbol;

// Represents the shared server state
class shared_state : public boost::enable_shared_from_this<shared_state> {
    std::string const doc_root_;
    std::string const topics_;
    enum class MessageType { Subscribe = 0, Unsubscribe = 1 };

public:
    explicit shared_state(std::string doc_root, std::string topics,
                          net::io_context &ioc_background_worker,
                          boost::program_options::variables_map vm,
                          boost::program_options::variables_map vm_env);

    std::string const &doc_root() const noexcept { return doc_root_; }
    net::io_context &ioc_background_worker_;
    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;
    std::map<std::string, boost::shared_ptr<symbol>> symbols_;
    boost::lockfree::spsc_queue<std::pair<std::string, std::string>,
                                boost::lockfree::capacity<1024>>
        spsc_queue_;
    boost::lockfree::spsc_queue<std::pair<parser::MsgType, std::string>,
                                boost::lockfree::capacity<1024>>
        spsc_queue_subscriber_;
    // Keep a list of all the connected clients
    std::unordered_set<websocket_session *> sessions_;
    parser parser_;
    sravz::router router_;
    boost::program_options::variables_map vm;
    boost::program_options::variables_map vm_env;
    std::vector<std::string> allowed_origins_;
    void init();
    void join(websocket_session *session);
    void websocket_subscribe_to_symbols(websocket_session *session,
                                        std::vector<std::string> topics);
    template <typename T>
    void websocket_unsubscribe_to_symbols(websocket_session *session, T topics);
    // void websocket_unsubscribe_symbols(websocket_session* session);
    void leave(websocket_session *session);
    void parse(std::string msg, websocket_session *session);
    // void send  (std::string message);
    void publish();
    void subscribe();
};

#endif
