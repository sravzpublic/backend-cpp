//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

//------------------------------------------------------------------------------
/*
    WebSocket chat server, multi-threaded

    This implements a multi-user chat room using WebSocket. The
    `io_context` runs on any number of threads, specified at
    the command line.

*/
//------------------------------------------------------------------------------

#include "webserver.hpp"

int webserver(std::string address_, std::string port_, std::string doc_root_,
              std::string threads_, std::string topics_,
              boost::program_options::variables_map vm,
              boost::program_options::variables_map vm_env) {
    // Check command line arguments.
    // if (argc != 6)
    // {
    //     std::cerr <<
    //         "Usage: websocket-chat-multi <address> <port> <doc_root>
    //         <threads> <topics>\n" << "Example:\n" << " websocket-chat-server
    //         0.0.0.0 8080 . 5 ETH-USD,BTC-USD\n";
    //     return EXIT_FAILURE;
    // }
    // auto address = net::ip::make_address(argv[1]);
    // auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    // auto doc_root = argv[3];
    // auto const threads = std::max<int>(1, std::atoi(argv[4]));
    // auto topics_ = argv[5];

    auto address = net::ip::make_address(address_.c_str());
    auto port = static_cast<unsigned short>(std::atoi(port_.c_str()));
    auto doc_root = doc_root_;
    auto const threads = std::max<int>(1, std::atoi(threads_.c_str()));
    auto topics = topics_;

    // The io_context is required for all I/O
    net::io_context ioc;
    net::io_context ioc_subscriber;
    net::io_context ioc_publisher;
    net::io_context ioc_background_worker;

    auto work = boost::asio::make_work_guard(ioc_background_worker);
    // Create and launch a listening port
    boost::shared_ptr<shared_state> shared_state_ =
        boost::make_shared<shared_state>(doc_root, topics,
                                         ioc_background_worker, vm, vm_env);
    boost::shared_ptr<listener> listener_ = boost::make_shared<listener>(
        ioc, tcp::endpoint{address, port}, shared_state_);
    boost::shared_ptr<subscriber> subscriber_ =
        boost::make_shared<subscriber>(ioc_subscriber, shared_state_);
    boost::shared_ptr<publisher> publisher_ =
        boost::make_shared<publisher>(ioc_publisher, shared_state_);

    // Start the sub-process
    listener_->run();
    // TODO: Fix the redis version error
    //  subscriber_->subscribe();
    publisher_->publish();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set signals(ioc, SIGINT, SIGTERM);

    signals.async_wait(
        [&ioc, &ioc_subscriber, &ioc_publisher,
         &ioc_background_worker](boost::system::error_code const &, int) {
            // Stop the io_context. This will cause run()
            // to return immediately, eventually destroying the
            // io_context and any remaining handlers in it.
            ioc.stop();
            // TODO: Fix the redis version error
            //  ioc_subscriber.stop();
            ioc_publisher.stop();
            ioc_background_worker.stop();
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    // For ioc and ioc_background_worker: times 2.
    v.reserve(threads * 2);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc] {
            BOOST_LOG_TRIVIAL(info) << "Starting ioc";
            ioc.run();
        });

    // Run background threads
    // std::vector<std::thread> v;
    // v.reserve(threads);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back([&ioc_background_worker] {
            BOOST_LOG_TRIVIAL(info) << "Starting background worker ioc";
            ioc_background_worker.run();
        });

    // Run redis subscriber io_context
    v.emplace_back([&ioc_subscriber] {
        BOOST_LOG_TRIVIAL(info) << "Starting ioc_subscriber";
        ioc_subscriber.run();
    });

    // Run websocket publisher io_context
    v.emplace_back([&ioc_publisher] {
        BOOST_LOG_TRIVIAL(info) << "Starting ioc_publisher";
        ioc_publisher.run();
    });

    // Block until all the threads exit
    for (auto &t : v)
        t.join();

    return EXIT_SUCCESS;
}
