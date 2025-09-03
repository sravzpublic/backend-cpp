/**
 * Sravz LLC
 * TODO: Make it Object Oriented
 * Use io_context instead of io_service
 * Use async handlers
 **/
#include <util.hpp>

boost::mutex global_stream_lock;

// Initialize the worker thread. Creates a redis connection per thread
// Connection is stored in a map with the key = thread ID
void initWorkerThread(
    boost::shared_ptr<boost::asio::io_service> io_service,
    boost::shared_ptr<
        std::map<boost::thread::id, boost::shared_ptr<RedisInstance>>>
        connectionPtrs) {
    // A global lock so the redis connection map can be updated with the
    // connection objects
    global_stream_lock.lock();
    std::cout << "[" << boost::this_thread::get_id() << "] Thread Start"
              << std::endl;
    // A map with threadID -> redis connection object
    // Each thread will use it's own resids connection since redis connection is
    // not thread safe
    boost::shared_ptr<RedisInstance> redis_ptr(
        new RedisInstance(getRedisConnectionOptions()));
    (*connectionPtrs)[boost::this_thread::get_id()] = redis_ptr;
    global_stream_lock.unlock();
    // Start the IO service, this will start the main thread
    io_service->run();

    // Unlock
    global_stream_lock.lock();
    std::cout << "[" << boost::this_thread::get_id() << "] Thread Finish"
              << std::endl;
    global_stream_lock.unlock();
}

// Process the message from the stream and send a response to the redis stream
void processMessage(
    std::string message,
    boost::shared_ptr<
        std::map<boost::thread::id, boost::shared_ptr<RedisInstance>>>
        redisConnection_ptrs) {
    // Declare a variable to hold the redis message
    auto key_response = "backend-node";
    std::vector<std::pair<std::string, std::string>> attrs_response = {
        {"f1",
         message +
             boost::lexical_cast<std::string>(boost::this_thread::get_id())},
        {"f2",
         message +
             boost::lexical_cast<std::string>(boost::this_thread::get_id())}};
    // Get the thread ID to be used to get the redis connection from the data
    // structure which holds the redis conneciton object
    auto thread_id = boost::this_thread::get_id();
    std::cout << "Sending response to node with ID by thread: " << thread_id
              << "\n";
    // Send the response to the redis stream
    auto response_id = (*redisConnection_ptrs)[thread_id]->xadd(
        key_response, "*", attrs_response.begin(), attrs_response.end());
    std::cout << "Response sent to node with ID: " << response_id << "\n";
}

// Runs Redis consumer. Listens to the Redis stream and pulls a message from the
// stream. Sends an ack to the stream so the message is deleted from the stream
void runRedisConsumer(
    boost::shared_ptr<boost::asio::io_service> io_service,
    boost::shared_ptr<RedisInstance> redis,
    boost::shared_ptr<
        std::map<boost::thread::id, boost::shared_ptr<RedisInstance>>>
        redisConnection_ptrs) {
    // Declare redis group name and other variables
    auto key = "backend-cpp";
    auto key_response = "backend-node";
    sw::redis::test::KeyDeleter<RedisInstance> deleter(*redis, key);
    auto group = "backend-cpp";
    auto consumer1 = "consumer1";

    // Declare variable to hold Redis Message
    std::vector<std::pair<std::string, std::string>> attrs = {{"f1", "v1"},
                                                              {"f2", "v2"}};
    auto id = redis->xadd(key, "*", attrs.begin(), attrs.end());
    auto keys = {std::make_pair(key, ">")};

    // Create redis consumer group backend-cpp on which this cpp program will
    // send the response
    try {
        redis->xgroup_create(key, group, "$", true);
    } catch (std::exception const &ex) {
        // BOOST_LOG_TRIVIAL(warning) << "Cannot create consumer group: " <<
        // ex.what();
        std::cout << "Cannot create consumer group: " << ex.what();
    }

    // std::chrono::milliseconds
    using Item =
        std::pair<std::string, std::unordered_map<std::string, std::string>>;
    using Result = std::unordered_map<std::string, std::vector<Item>>;
    // Create a timeer, redis client will listen to the messages for this amount
    // of time and then restart the listen loop
    auto timeout =
        std::chrono::milliseconds(10000); // Default wait for 1 second

    // IO Service will block in this while loop and keep polling for the
    // messages
    while (true) {
        Result result;
        redis->xreadgroup(
            group,
            boost::lexical_cast<std::string>(boost::this_thread::get_id()), key,
            std::string(">"),
            timeout, // Timeout milliseconds - polls every second
            1,
            std::inserter(
                result,
                result.end())); // Add the response to the result variable
        if (result.size() > 0) {
            std::cout << result.size() << " " << result[key][0].second["f1"]
                      << " " << result[key][0].second["f2"] << "\n";
            auto acked_id = redis->xack(key, group, result[key][0].first);
            auto delete_id = redis->xdel(key, result[key][0].first);
            // Ack the redis message so it will be deleted from the redis stream
            std::cout << "Acked ID: " << acked_id << "\n";
            if (acked_id > 0) {
                timeout = std::chrono::milliseconds(0);
            } else {
                timeout = std::chrono::milliseconds(1000);
            }
            // Post the message to the io_service so some thread will process
            // and message and send the response
            io_service->post(boost::bind(processMessage,
                                         result[key][0].second["f1"],
                                         redisConnection_ptrs));
            // Empty the result and reuse the variable
            result.erase(result.begin(), result.end());
        }
    }
}

int main(int argc, char **argv) {
    // Declare the IO Service
    boost::shared_ptr<boost::asio::io_service> io_service(
        new boost::asio::io_service);
    // Add empty work to IO Service so the IO Service does not exit when started
    boost::shared_ptr<boost::asio::io_service::work> work(
        new boost::asio::io_service::work(*io_service));

    // Dispaly IO Service Thread ID
    global_stream_lock.lock();
    std::cout << "[" << boost::this_thread::get_id()
              << "] The program will exit when all work has finished."
              << std::endl;
    global_stream_lock.unlock();

    // Create Redis Connection Instance
    using RedisInstance = sw::redis::Redis;
    RedisInstance redis(getRedisConnectionOptions());

    // Make the Redis Connection Instance a shared pointer so it could be passed
    // around
    boost::shared_ptr<RedisInstance> redis_ptr(
        new RedisInstance(getRedisConnectionOptions()));
    boost::shared_ptr<
        std::map<boost::thread::id, boost::shared_ptr<RedisInstance>>>
        redisConnection_ptrs(
            new std::map<boost::thread::id, boost::shared_ptr<RedisInstance>>);

    // Created Redis Stream Group Key IDs
    auto key = "backend-cpp";
    auto group = "backend-cpp";
    try {
        redis.xgroup_create(key, group, "$", true);
    } catch (std::exception const &ex) {
        // BOOST_LOG_TRIVIAL(warning) << "Cannot create consumer group: " <<
        // ex.what();
        std::cout << "Cannot create consumer group: " << ex.what() << "\n";
    }

    // Create some pthreads which join the IO Service and perform the work
    int MAX_NUMBER_OF_THREADS = 2;
    std::string NUM_THREADS;
    if (getenv("NUM_THREADS", NUM_THREADS))
        MAX_NUMBER_OF_THREADS = std::stoi(NUM_THREADS);
    boost::thread_group worker_threads;
    std::cout << "Starting: " << NUM_THREADS << " workers"
              << "\n";
    for (int x = 0; x < MAX_NUMBER_OF_THREADS; ++x) {
        worker_threads.create_thread(
            boost::bind(&initWorkerThread, io_service, redisConnection_ptrs));
    }

    // Pass the IO service to the Redis Consumer function that will start the IO
    // Service
    runRedisConsumer(io_service, redis_ptr, redisConnection_ptrs);

    // Wait for the threads to join
    worker_threads.join_all();

    return 0;
}