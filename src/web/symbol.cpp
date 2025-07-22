//
// Sravz LLC
//

#include "symbol.hpp"

symbol::symbol(std::string code, std::string quote,
               std::chrono::milliseconds::rep time)
    : code(std::move(code)), quote(quote), time(time), refcount(0) {}

void symbol::join(websocket_session *session) {
    sessions_.insert(session);
}

int symbol::leave(websocket_session *session) {
    return sessions_.erase(session);
}

// // Broadcast a message to all websocket client sessions
// void
// symbol::
// send(std::string message)
// {
//     // Put the message in a shared pointer so we can re-use it for each
//     client auto const ss = boost::make_shared<std::string
//     const>(std::move(message));

//     // Make a local list of all the weak pointers representing
//     // the sessions, so we can do the actual sending without
//     // holding the mutex:
//     std::vector<boost::weak_ptr<websocket_session>> v;
//     {
//         std::lock_guard<std::mutex> lock(mutex_);
//         v.reserve(sessions_.size());
//         for(auto p : sessions_)
//             v.emplace_back(p->weak_from_this());
//     }

//     // For each session in our local list, try to acquire a strong
//     // pointer. If successful, then send the message on that session.
//     for(auto const& wp : v)
//         if(auto sp = wp.lock())
//             sp->send(ss);
// }
