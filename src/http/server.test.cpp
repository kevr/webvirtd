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
#include <http/client.hpp>
#include <http/server.hpp>
#include <syscall.hpp>
#include <util/config.hpp>
#include <util/util.hpp>

#include <boost/beast/http/status.hpp>
#include <gtest/gtest.h>
#include <thread>

using namespace webvirt;

using testing::Test;

class server_test : public Test
{
protected:
    static std::filesystem::path tmpdir, socket_path;

    http::io_context io;
    std::shared_ptr<http::server> server;

    http::io_context client_io;
    std::shared_ptr<http::client> client;

    config conf;

public:
    static void SetUpTestSuite()
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
    }

    void SetUp() override
    {
        syscall::ref().fs_remove(socket_path);
        std::cout << socket_path << std::endl;

        setup_config();
        config::change(conf);

        server = std::make_shared<webvirt::http::server>(io, socket_path);
        client =
            std::make_shared<http::client>(client_io, socket_path.string());
    }

    void TearDown() override
    {
        config::reset();
        server.reset();
    }

    static void TearDownTestSuite()
    {
        syscall::ref().fs_remove_all(tmpdir);
    }

private:
    void setup_config()
    {
        conf.add_option("threads",
                        boost::program_options::value<unsigned>()
                            ->default_value(1)
                            ->multitoken(),
                        "number of worker threads");

        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);
    }
};

std::filesystem::path server_test::tmpdir, server_test::socket_path;

TEST_F(server_test, standalone_io)
{
    auto standalone_socket_path = tmpdir / "standalone.sock";
    http::server standalone_server(standalone_socket_path.string());
}

TEST_F(server_test, runs_with_defaults)
{
    auto server_thread = std::thread([&] {
        server->on_close([&] {
            io.stop();
        });
        server->run();
    });

    client->async_get("/").run();

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

    http::response response;
    client->on_response([&response](const auto &response_) {
        response = response_;
    });
    client->async_get("/").run();

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

    EXPECT_EQ(client->host(), "localhost");
    EXPECT_EQ(client->version(), webvirt::http::version::http_1_1);

    http::response response;
    client->on_response([&response](const auto &response_) {
        response = response_;
    });
    client->async_post("/").run();

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
        server->on_accept([](auto conn) {
            conn->close();
        });
        server->run();
    });

    client->async_get("/").run();

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

    client->on_connect([](auto &client) {
        client.close();
    });
    client->async_get("/").run();

    server_thread.join();
}

TEST_F(server_test, write_error)
{
    auto server_thread = std::thread([&] {
        server->on_error([this](const char *, boost::beast::error_code) {
            io.stop();
        });
        server->on_request([&](auto conn, const auto &, auto &) {
            conn->close();
        });
        server->run();
    });

    client->on_error([&](const char *, boost::beast::error_code) {
        client_io.stop();
    });
    client->async_get("/").run();

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
    client_io.run();

    server_thread.join();
}
