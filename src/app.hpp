#ifndef APP_HPP
#define APP_HPP

#include "http/router.hpp"
#include "http/server.hpp"
#include <regex>

namespace webvirt
{

class app
{
private:
    http::router router_;

    webvirt::io_service &io_;
    http::server<net::unix> server_;

public:
    app(webvirt::io_service &io, const std::filesystem::path &socket_path);
    std::size_t run();

private:
    template <typename Func>
    auto bind(Func fn)
    {
        return std::bind(fn,
                         this,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3);
    }

    template <typename Func>
    auto bind_user(Func fn)
    {
        return std::bind(fn,
                         this,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3,
                         std::placeholders::_4);
    }

private:
    void append_trailing_slash(
        const std::smatch &,
        const beast::http::request<beast::http::dynamic_body> &,
        beast::http::response<beast::http::string_body> &);
    void domains(const std::string &, const std::smatch &,
                 const beast::http::request<beast::http::dynamic_body> &,
                 beast::http::response<beast::http::string_body> &);
    void domain(const std::string &, const std::smatch &,
                const beast::http::request<beast::http::dynamic_body> &,
                beast::http::response<beast::http::string_body> &);
};

}; // namespace webvirt

#endif /* APP_HPP */
