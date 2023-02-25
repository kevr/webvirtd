/*
 * Copyright 2023 Kevin Morris
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>
#include <util/logging.hpp>
#include <util/util.hpp>

#include <boost/beast.hpp>
#include <memory>

namespace webvirt::http
{

/** An HTTP client used for testing purposes */
class client : public std::enable_shared_from_this<client>
{
protected:
    std::string socket_path_;

    io_context &io_;
    net::unix::socket socket_;
    boost::beast::flat_buffer buffer_ { 8192 };
    beast::http::request<beast::http::string_body> request_;
    beast::http::response<beast::http::string_body> response_;

    std::string host_;
    int version_;

    handler<client &> on_connect_;
    handler<const http::response &> on_response_;
    handler<const char *, beast::error_code> on_error_;
    handler<> on_close_;

public:
    /** Construct an HTTP client
     *
     * @param io webvirt::http::io_context
     * @param socket_path Path to unix socket
     * @param host Host header
     * @param version webvirt::http::version
     **/
    explicit client(io_context &io, std::string socket_path,
                    std::string host = "localhost",
                    int version = webvirt::http::version::http_1_1);

    /** Set the value used for HTTP Host header
     *
     * @param value New HTTP Host header value
     * @returns Reference to this
     **/
    client &host(std::string value);

    /** Returns the Host header value
     *
     * @returns Internal Host header value
     **/
    const std::string &host() const;

    /** Set the HTTP version
     *
     * @param version Version to set; see webvirt::http::version
     * @returns Reference to this
     **/
    client &version(int version);

    /** Returns the HTTP version
     *
     * @returns Internal HTTP version value
     **/
    int version() const;

    /** Begin an OPTIONS request toward `target`
     *
     * @param target Target request URI
     * @returns Reference to this
     **/
    client &async_options(const char *target);

    /** Begin a GET request toward `target`
     *
     * @param target Target request URI
     * @returns Reference to this
     **/
    client &async_get(const char *target);

    /** Begin a POST request toward `target` with POST data
     *
     * @param target Target request URI
     * @param data Optional POST data sent with the request
     * @returns Reference to this
     **/
    client &async_post(const char *target,
                       const std::string &data = std::string());

    /** Returns internal HTTP request
     *
     * @returns Reference to internal HTTP request
     **/
    const beast::http::request<beast::http::string_body> &request() const;

    /** Close the client */
    void close();

    /** Run the client's webvirt::http::io_context
     *
     * @returns Number of handlers processed
     **/
    std::size_t run();

    handler_setter(on_connect, on_connect_);
    handler_setter(on_response, on_response_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void init_request(const char *target);
    void async_connect();

    void client_on_connect(boost::beast::error_code ec);
    void client_on_write(boost::beast::error_code ec, std::size_t bytes);
    void client_on_read(boost::beast::error_code ec, std::size_t bytes);
};

}; // namespace webvirt::http

#endif /* HTTP_CLIENT_HPP */
