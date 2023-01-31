/* SPDX-License-Identifier: Apache 2.0 */
#include "app.hpp"
#include "http/client.hpp"
#include "util.hpp"
#include <gtest/gtest.h>
#include <thread>
using namespace webvirt;
using testing::Test;

class app_test : public Test
{
protected:
    std::filesystem::path tmpdir, socket_path;

    webvirt::io_service io_;
    std::shared_ptr<webvirt::app> app_;
    std::thread server_thread;

    using client_t = http::client<net::unix>;
    webvirt::io_service client_io_;
    std::shared_ptr<client_t> client;

public:
    void SetUp() override
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
        app_ = std::make_shared<webvirt::app>(io_, socket_path);
        client = std::make_shared<client_t>(client_io_, socket_path);

        server_thread = std::thread([&] {
            app_->run();
        });
    }

    void TearDown() override
    {
        server_thread.join();
    }
};

TEST_F(app_test, method_not_allowed)
{
    http::response response;
    client->on_response([&response, this](const auto &response_) {
        response = response_;
        io_.stop();
    });
    client->async_options("/domains/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::method_not_allowed));
}

TEST_F(app_test, not_found)
{
    http::response response;
    client->on_response([&response, this](const auto &response_) {
        response = response_;
        io_.stop();
    });
    client->async_get("/not-found/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
}
