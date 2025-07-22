//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/CppCon2018
//

#include "http_session.hpp"

#include "jwt/jwt.hpp" // for JWT token validation includes implementation
using namespace std;

//------------------------------------------------------------------------------

// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path) {
    using beast::iequals;
    auto const ext = [&path] {
        auto const pos = path.rfind(".");
        if (pos == beast::string_view::npos)
            return beast::string_view{};
        return path.substr(pos);
    }();
    if (iequals(ext, ".htm"))
        return "text/html";
    if (iequals(ext, ".html"))
        return "text/html";
    if (iequals(ext, ".php"))
        return "text/html";
    if (iequals(ext, ".css"))
        return "text/css";
    if (iequals(ext, ".txt"))
        return "text/plain";
    if (iequals(ext, ".js"))
        return "application/javascript";
    if (iequals(ext, ".json"))
        return "application/json";
    if (iequals(ext, ".xml"))
        return "application/xml";
    if (iequals(ext, ".swf"))
        return "application/x-shockwave-flash";
    if (iequals(ext, ".flv"))
        return "video/x-flv";
    if (iequals(ext, ".png"))
        return "image/png";
    if (iequals(ext, ".jpe"))
        return "image/jpeg";
    if (iequals(ext, ".jpeg"))
        return "image/jpeg";
    if (iequals(ext, ".jpg"))
        return "image/jpeg";
    if (iequals(ext, ".gif"))
        return "image/gif";
    if (iequals(ext, ".bmp"))
        return "image/bmp";
    if (iequals(ext, ".ico"))
        return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff"))
        return "image/tiff";
    if (iequals(ext, ".tif"))
        return "image/tiff";
    if (iequals(ext, ".svg"))
        return "image/svg+xml";
    if (iequals(ext, ".svgz"))
        return "image/svg+xml";
    return "application/text";
}

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path) {
    if (base.empty())
        return std::string(path);
    std::string result(base);
#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto &c : result)
        if (c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}

template <class Body, class Allocator>
boost::optional<http::message_generator> http_session::validate_jwt_token(
    http::request<Body, http::basic_fields<Allocator>> &req) {
    if (req.target() == "/health" || req.method() == http::verb::options)
        return boost::none;

    // Returns a bad request response
    auto const unauthorized = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::unauthorized,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Extract the Bearer token from the Authorization header
    std::string bearer_token;
    if (req.count(boost::beast::http::field::authorization) > 0) {
        std::string authorization_header =
            req[boost::beast::http::field::authorization];
        if (authorization_header.substr(0, 7) == "Bearer ") {
            bearer_token = authorization_header.substr(7);
            try {
                auto dec_obj = jwt::decode(bearer_token,
                                           jwt::params::algorithms({"HS256"}),
                                           jwt::params::secret(jwt_token));
                return boost::none;
            } catch (const jwt::TokenExpiredError &e) {
                return boost::make_optional<http::message_generator>(
                    unauthorized("Unauthorized - api_token expired"));
            } catch (const jwt::SignatureFormatError &e) {
                return boost::make_optional<http::message_generator>(
                    unauthorized(
                        "Unauthorized - api_token signature format error"));
            } catch (const jwt::DecodeError &e) {
                return boost::make_optional<http::message_generator>(
                    unauthorized("Unauthorized - api_token decode error"));
            } catch (const jwt::VerificationError &e) {
                return boost::make_optional<http::message_generator>(
                    unauthorized(
                        "Unauthorized - api_token verification error"));
            } catch (...) {
                return boost::make_optional<http::message_generator>(
                    unauthorized("Unauthorized - api_token unknown error"));
            }
        }
    }

    return boost::make_optional<http::message_generator>(
        unauthorized("Unauthorized - api_token missing"));
}

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.
template <class Body, class Allocator>
boost::optional<http::message_generator> http_session::handle_request(
    beast::string_view doc_root,
    http::request<Body, http::basic_fields<Allocator>> &&req) {
    // Returns a bad request response
    auto const bad_request = [&req](beast::string_view why) {
        http::response<http::string_body> res{http::status::bad_request,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };

    // Returns a not found response
    auto const not_found = [&req](beast::string_view target) {
        http::response<http::string_body> res{http::status::not_found,
                                              req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() =
            "The resource '" + std::string(target) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error = [&req](beast::string_view what) {
        http::response<http::string_body> res{
            http::status::internal_server_error, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "An error occurred: '" + std::string(what) + "'";
        res.prepare_payload();
        return res;
    };

    // Make sure we can handle the method
    if (req.method() != http::verb::get && req.method() != http::verb::head &&
        req.method() != http::verb::post && req.method() != http::verb::put &&
        req.method() != http::verb::delete_ &&
        req.method() != http::verb::patch &&
        req.method() != http::verb::options)
        return boost::make_optional<http::message_generator>(
            bad_request("Unknown HTTP-method"));

    // Cache the size since we need it after the move
    auto const size = 1; // body.size();
    std::string path = req.target();
    auto const head_response = [&req, &size, &path]() {
        http::response<http::empty_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(path));
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return res;
    };

    if (req.method() == http::verb::head) {
        return boost::make_optional<http::message_generator>(head_response());
    }

    auto const options_response = [session = shared_from_this(), &req, &size,
                                   &path]() {
        http::response<http::empty_body> res{http::status::no_content,
                                             req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);

        auto originHeader = req.find(boost::beast::http::field::origin);
        if (originHeader != req.end()) {
            std::string origin(originHeader->value().data(),
                               originHeader->value().size());
            // Process the origin value as needed
            BOOST_LOG_TRIVIAL(debug)
                << "CORS Request Origin: " << origin << std::endl;
            auto it =
                std::find(session->state_->allowed_origins_.begin(),
                          session->state_->allowed_origins_.end(), origin);
            if (it != session->state_->allowed_origins_.end()) {
                BOOST_LOG_TRIVIAL(debug) << "Origin set to: " << origin;
                res.set(http::field::access_control_allow_origin, origin);
            }
        } else {
            BOOST_LOG_TRIVIAL(debug) << "CORS: Host not found in the header";
        }

        res.set(
            http::field::access_control_allow_headers,
            "Origin,X-Requested-With,Content-Type,Accept,Authorization,Accept-"
            "Language,Content-Language,Last-Event-ID,X-HTTP-Method-Override");
        res.set(http::field::access_control_allow_methods,
                "POST, GET, PUT, DELETE, PATCH, HEAD");
        res.set(http::field::access_control_allow_credentials, "true");
        res.keep_alive(req.keep_alive());
        return res;
    };

    if (req.method() == http::verb::options) {
        return boost::make_optional<http::message_generator>(
            options_response());
    }
    // Respond to HEAD request and health check
    auto const health_check_response = [&req, &size, &path]() {
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.body() = "Ok";
        res.prepare_payload();
        return res;
    };

    if (path == "/health") {
        return boost::make_optional<http::message_generator>(
            health_check_response());
    }

    // Let the background worker generate the response
    boost::asio::post(
        state_->ioc_background_worker_.get_executor(),
        [session = shared_from_this(), req = std::move(req)]() {
            try {
                // BOOST_LOG_TRIVIAL(info)
                //     << "Processing in the background thread";
                auto [code, body] = session->state_->router_.route(
                    req.method(), req.base().target(), req.body());
                http::response<http::string_body> res{code, req.version()};
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.set(http::field::content_type, "application/json");
                res.keep_alive(req.keep_alive());
                res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
                res.body() = body;
                res.content_length(res.body().size());
                /* Set cors */
                auto originHeader = req.find(boost::beast::http::field::origin);
                if (originHeader != req.end()) {
                    std::string origin(originHeader->value().data(),
                                       originHeader->value().size());
                    // Process the origin value as needed
                    BOOST_LOG_TRIVIAL(debug)
                        << "CORS Request Origin: " << origin << std::endl;
                    auto it = std::find(
                        session->state_->allowed_origins_.begin(),
                        session->state_->allowed_origins_.end(), origin);
                    if (it != session->state_->allowed_origins_.end()) {
                        BOOST_LOG_TRIVIAL(debug) << "Origin set to: " << origin;
                        res.set(http::field::access_control_allow_origin,
                                origin);
                    }
                } else {
                    BOOST_LOG_TRIVIAL(warning)
                        << "CORS: Host not found in the header";
                }
                boost::asio::post(
                    session->stream_.get_executor(),
                    boost::bind(&http_session::do_write, session, res));
            } catch (const std::exception &ex) {
                BOOST_LOG_TRIVIAL(warning)
                    << "Error in background thread: cannot send message"
                    << ex.what();
            }
        });

    return boost::none;
}

//------------------------------------------------------------------------------

http_session::http_session(tcp::socket &&socket,
                           boost::shared_ptr<shared_state> const &state)
    : stream_(std::move(socket)), state_(state) {
    jwt_token = state_->vm_env["JWT_ACCESS_TOKEN_SECRET"].as<string>();
    disableJWTTokenValidation =
        state_->vm_env["DisableJWTTokenValidation"].as<string>();
}

void http_session::run() {
    do_read();
}

// Report a failure
void http_session::fail(beast::error_code ec, char const *what) {
    // Don't report on canceled operations
    if (ec == net::error::operation_aborted)
        return;

    std::cerr << what << ": " << ec.message() << "\n";
}

void http_session::do_read() {
    // Construct a new parser for each message
    parser_.emplace();

    // Apply a reasonable limit to the allowed size
    // of the body in bytes to prevent abuse.
    parser_->body_limit(10000);

    // Set the timeout.
    stream_.expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(
        stream_, buffer_, parser_->get(),
        beast::bind_front_handler(&http_session::on_read, shared_from_this()));
}

void http_session::on_read(beast::error_code ec, std::size_t) {
    // This means they closed the connection
    if (ec == http::error::end_of_stream) {
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Handle the error, if any
    if (ec)
        return fail(ec, "read");

    bool keep_alive;
    auto self = shared_from_this();

    // boost::optional<http::message_generator> unauthorized_res =
    // validate_jwt_token(parser_->release());

    if (disableJWTTokenValidation != "true") {
        if (boost::optional<http::message_generator> unauthorized_res =
                validate_jwt_token(parser_->get())) {
            BOOST_LOG_TRIVIAL(warning) << "User Unauthorized";
            keep_alive = unauthorized_res->keep_alive();
            // Send the response
            beast::async_write(
                stream_, std::move(*unauthorized_res),
                [self, keep_alive](beast::error_code ec, std::size_t bytes) {
                    self->on_write(ec, bytes, keep_alive);
                });
            return;
        } else {
            BOOST_LOG_TRIVIAL(info) << "User Authorized";
            // std::cout << "User Authorized\n";
        }
    } else {
        BOOST_LOG_TRIVIAL(warning) << "JWT Validation Disabled!";
    }

    // See if it is a WebSocket Upgrade
    if (websocket::is_upgrade(parser_->get())) {
        // Create a websocket session, transferring ownership
        // of both the socket and the HTTP request.
        boost::make_shared<websocket_session>(stream_.release_socket(), state_)
            ->run(parser_->release());
        return;
    }

    if (boost::optional<http::message_generator> msg =
            handle_request(state_->doc_root(), parser_->release())) {
        // Determine if we should close the connection
        keep_alive = msg->keep_alive();
        // Send the response
        beast::async_write(
            stream_, std::move(*msg),
            [self, keep_alive](beast::error_code ec, std::size_t bytes) {
                self->on_write(ec, bytes, keep_alive);
            });
        return;
    } // else {
        // BOOST_LOG_TRIVIAL(info) << "Sent the request to background worker";
    // }
}

void http_session::do_write(http::response<http::string_body> &res) {
    auto keep_alive = res.keep_alive();
    auto msg = res.body();
    res.body() = msg + " webserver thread ID: " +
        boost::lexical_cast<std::string>(boost::this_thread::get_id());
    http::message_generator gen(std::move(res));
    beast::async_write(
        stream_, std::move(gen),
        [this, keep_alive](beast::error_code ec, std::size_t bytes) {
            this->on_write(ec, bytes, keep_alive);
        });
    return;
}

void http_session::on_write(beast::error_code ec, std::size_t,
                            bool keep_alive) {
    // Handle the error, if any
    if (ec)
        return fail(ec, "write");

    if (!keep_alive) {
        // This means we should close the connection, usually because
        // the response indicated the "Connection: close" semantic.
        stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
        return;
    }

    // Read another request
    // do_read();
}
