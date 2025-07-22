//
// SRAVZ LLC
//

#ifndef SRAVZ_WEB_SUBSCRIBER_HPP
#define SRAVZ_WEB_SUBSCRIBER_HPP

#include <util.hpp>

#include "shared_state.hpp"

// Represents the shared server state
class subscriber : public boost::enable_shared_from_this<subscriber> {
    // RedisInstance redis_consumer_;
    net::io_context &ioc_subscriber_;
    boost::shared_ptr<shared_state> state_;
    // Single redis subscriber TODO: Make it multiple
    boost::shared_ptr<RedisInstance> redis_ptr_;

public:
    explicit subscriber(net::io_context &ioc_subscriber,
                        boost::shared_ptr<shared_state> const &state);
    void subscribe();
};

#endif
