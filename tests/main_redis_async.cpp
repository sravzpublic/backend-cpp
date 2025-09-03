/**
 * Sravz LLC
 * TODO: Handle multiple topics
 **/
#include <redis_subscriber.hpp>
#include <util.hpp>

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Usage: sravz_redis_async <topic>\n"
                  << "Example:\n"
                  << "    sravz_redis_async ETH-USD,BTC-USD\n";
        return EXIT_FAILURE;
    }
    auto const topics = argv[1];

    auto io = std::make_shared<boost::asio::thread_pool>(3);

    // Singal handler to exit cleanly
    boost::asio::signal_set signals(*io, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto) {
        io->stop();
        std::cout << "Signal received, stopped the process.";
    });

    std::make_shared<RedisSubscriber>(topics, io)->run();

    io->join();
}