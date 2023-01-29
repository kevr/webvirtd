#include "server.hpp"
#include "../syscaller.hpp"
#include "../util.hpp"
#include "client.hpp"
#include <boost/beast/http/status.hpp>
#include <gtest/gtest.h>
#include <thread>

using testing::Test;

class server_test : public Test
{
protected:
    static std::filesystem::path tmpdir, socket_path;

    webvirt::io_service io;
    webvirt::io_service client_io;

    using server_t = webvirt::http::server<webvirt::net::unix>;
    std::shared_ptr<server_t> server;

public:
    static void SetUpTestSuite()
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
    }

    void SetUp() override
    {
        auto &sys = webvirt::syscaller::instance();
        sys.fs_remove(socket_path);
        std::cout << socket_path << std::endl;

        server = std::make_shared<webvirt::http::server<webvirt::net::unix>>(
            io, socket_path);
    }

    void TearDown() override
    {
        server.reset();
    }

    static void TearDownTestSuite()
    {
        auto &sys = webvirt::syscaller::instance();
        sys.fs_remove_all(tmpdir);
    }
};

std::filesystem::path server_test::tmpdir, server_test::socket_path;

TEST_F(server_test, standalone_io)
{
    auto standalone_socket_path = tmpdir / "standalone.sock";
    server_t standalone_server(standalone_socket_path.string());
}

TEST_F(server_test, runs_with_defaults)
{
    auto server_thread = std::thread([&] {
        server->on_close([&] {
            io.stop();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());

    client->async_get("/");
    client_io.process();

    server_thread.join();
}

TEST_F(server_test, get)
{
    auto server_thread = std::thread([&] {
        server->on_close([&] {
            io.stop();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());

    boost::beast::http::response<boost::beast::http::string_body> response;
    client->on_response([&response](const auto &response_) {
        response = response_;
    });
    client->async_get("/");
    client_io.process();

    server_thread.join();

    auto request = client->request();
    std::cout << request;

    std::cout << response;
    EXPECT_EQ(response.version(), webvirt::http::version::http_1_1);
    EXPECT_EQ(response.result_int(),
              static_cast<int>(boost::beast::http::status::ok));
    EXPECT_EQ(response.has_content_length(), true);
    EXPECT_EQ(response.at("connection"), "close");
    EXPECT_EQ(response.at("server"), BOOST_BEAST_VERSION_STRING);
}

TEST_F(server_test, post)
{
    auto server_thread = std::thread([&] {
        server->on_close([&] {
            io.stop();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());
    EXPECT_EQ(client->host(), "localhost");
    EXPECT_EQ(client->version(), webvirt::http::version::http_1_1);

    boost::beast::http::response<boost::beast::http::string_body> response;
    client->on_response([&response](const auto &response_) {
        response = response_;
    });
    client->async_post("/");
    client_io.process();

    server_thread.join();

    auto request = client->request();
    std::cout << request;

    std::cout << response;
    EXPECT_EQ(response.version(), webvirt::http::version::http_1_1);
    EXPECT_EQ(response.result_int(),
              static_cast<int>(boost::beast::http::status::ok));
    EXPECT_EQ(response.has_content_length(), true);
    EXPECT_EQ(response.at("connection"), "close");
    EXPECT_EQ(response.at("server"), BOOST_BEAST_VERSION_STRING);
}

TEST_F(server_test, client_connect_error)
{
    auto server_thread = std::thread([&] {
        server->on_error([&](const char *, boost::beast::error_code) {
            io.stop();
        });
        server->on_accept([&](auto &conn) {
            conn.close();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());
    client->async_get("/");
    client_io.process();

    server_thread.join();
}

TEST_F(server_test, read_error)
{
    auto server_thread = std::thread([&] {
        server->on_error([&](const char *, boost::beast::error_code) {
            io.stop();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());
    client->on_connect([](auto &client) {
        client.close();
    });
    client->async_get("/");
    client_io.process();

    server_thread.join();
}

TEST_F(server_test, write_error)
{
    auto server_thread = std::thread([&] {
        server->on_error([this](const char *, boost::beast::error_code) {
            io.stop();
        });
        server->on_request([&](auto &conn, const auto &, auto &) {
            conn.close();
        });
        server->run();
    });

    using client_t = webvirt::http::client<webvirt::net::unix>;
    std::shared_ptr<client_t> client =
        std::make_shared<client_t>(client_io, socket_path.string());
    client->on_error([&](const char *, boost::beast::error_code) {
        client_io.stop();
    });
    client->async_get("/");
    client_io.process();

    server_thread.join();
}

TEST_F(server_test, deadline)
{
    using namespace webvirt;

    auto server_thread = std::thread([&] {
        server->timeout(std::chrono::milliseconds(10));
        server->on_close([&] {
            io.stop();
        });
        server->run();
    });

    net::unix::socket socket_(client_io);
    socket_.async_connect(socket_path.string(), [](boost::beast::error_code) {
    });
    client_io.run_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    client_io.process();

    server_thread.join();
}
