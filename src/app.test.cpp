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
#include "util/signal.hpp"
#include <app.hpp>
#include <http/client.hpp>
#include <http/handlers.hpp>
#include <mocks/libvirt.hpp>
#include <util/config.hpp>
#include <util/retry.hpp>
#include <util/util.hpp>
#include <ws/client.hpp>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <json/json.h>
#include <thread>
#include <tuple>

using namespace webvirt;
using namespace std::string_literals;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class tmpdir_test : public Test
{
protected:
    std::filesystem::path tmpdir, socket_path;
    uid_t uid;
    std::string username;

public:
    void SetUp() override
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";

        auto &sys = syscall::ref();
        uid = sys.getuid();
        auto *passwd = sys.getpwuid(uid);
        username = passwd->pw_name;

        auto &conf = config::ref();
        conf.add_option("threads",
                        boost::program_options::value<unsigned>()
                            ->default_value(1)
                            ->multitoken(),
                        "number of worker threads");
        conf.add_option("libvirt-shutdown-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.01)
                            ->multitoken(),
                        "libvirt shutdown timeout");
        conf.add_option("libvirt-shutoff-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.02)
                            ->multitoken(),
                        "libvirt shutoff timeout");

        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);
    }

    void TearDown() override
    {
        syscall::ref().fs_remove_all(tmpdir);
    }
};

class app_test : public tmpdir_test
{
protected:
    http::io_context io_;
    std::shared_ptr<webvirt::app> app_;
    std::thread server_thread;

    http::io_context client_io_;
    std::shared_ptr<http::client> client;

    http::response response;

public:
    void SetUp() override
    {
        tmpdir_test::SetUp();

        app_ = std::make_shared<webvirt::app>(io_, socket_path);
        client = std::make_shared<http::client>(client_io_, socket_path);

        client->on_response([this](const auto &response_) {
            response = response_;
            io_.stop();
        });

        server_thread = std::thread([&] {
            app_->run();
        });
    }

    void TearDown() override
    {
        server_thread.join();
        tmpdir_test::TearDown();
    }
};

class mock_app_test : public app_test
{
protected:
    connect_ptr conn;
    mocks::libvirt lv;

    // libvirt control
    bool libvirt_closed = false;
    std::function<void(webvirt::connect *, int, void *)> libvirt_close;
    std::function<void(void *)> libvirt_free;

public:
    void SetUp() override
    {
        app_test::SetUp();
        libvirt::change(lv);

        conn = std::make_shared<webvirt::connect>();

        auto &conf = config::ref();
        conf.add_option("libvirt-shutdown-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.01)
                            ->multitoken(),
                        "libvirt shutdown timeout");
        conf.add_option("libvirt-shutoff-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.02)
                            ->multitoken(),
                        "libvirt shutoff timeout");

        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);

        EXPECT_CALL(lv, virConnectRegisterCloseCallback(_, _, _, _))
            .WillRepeatedly(
                Invoke([&](connect_ptr,
                           void (*close_fn)(webvirt::connect *, int, void *),
                           void *closed_ptr,
                           void (*free_fn)(void *)) {
                    libvirt_closed = *reinterpret_cast<bool *>(closed_ptr);
                    libvirt_close = close_fn;
                    libvirt_free = free_fn;
                    return 0;
                }));

        logger::enable_debug(true);
    }

    void TearDown() override
    {
        logger::reset_debug();
        app_test::TearDown();
        libvirt::reset();
    }
};

class websocket_test : public tmpdir_test
{
protected:
    mocks::libvirt lv;

    http::io_context io_;
    std::shared_ptr<webvirt::app> app_;
    std::thread server_thread;

    http::io_context client_io_;
    std::shared_ptr<websocket::client> client;

public:
    void SetUp() override
    {
        tmpdir_test::SetUp();
        logger::enable_debug(true);

        libvirt::change(lv);
        auto conn = std::make_shared<webvirt::connect>();
        EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));
        EXPECT_CALL(lv, virEventRegisterDefaultImpl());
        EXPECT_CALL(lv, virConnectRegisterCloseCallback(_, _, _, _))
            .WillOnce(Return(0));

        app_ = std::make_shared<webvirt::app>(io_, socket_path);
        app_->server().on_close([this] {
            io_.stop();
        });
        server_thread = std::thread([this] {
            app_->run();
        });
        client = std::make_shared<websocket::client>(client_io_,
                                                     socket_path.c_str());
    }

    void TearDown() override
    {
        server_thread.join();
        libvirt::reset();
        logger::reset_debug();
        tmpdir_test::TearDown();
    }
};

TEST_F(tmpdir_test, event_registration_fails)
{
    testing::internal::CaptureStderr();

    mocks::libvirt lv;
    libvirt::change(lv);

    EXPECT_CALL(lv, virEventRegisterDefaultImpl()).WillOnce(Return(-1));

    http::io_context io;
    auto app = std::make_shared<webvirt::app>(io, socket_path);
    app->run();

    libvirt::reset();

    auto output = testing::internal::GetCapturedStderr();
    EXPECT_NE(
        output.find("Event loop encountered an error during registration"),
        std::string::npos);
}

TEST_F(app_test, method_not_allowed)
{
    const char *endpoint = "/users/test/domains/";
    client->async_post(endpoint).run();

    EXPECT_EQ(response.result(), beast::http::status::method_not_allowed);
}

TEST_F(app_test, options)
{
    const char *endpoint = "/users/test/domains/";
    client->async_options(endpoint).run();

    EXPECT_EQ(response.at(beast::http::field::allow), "GET, OPTIONS");
}

TEST_F(app_test, options_not_found)
{
    const char *endpoint = "/users/test/blahblah/";
    client->async_options(endpoint).run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);
}

TEST_F(app_test, not_found)
{
    client->async_get("/not-found/").run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);
}

TEST_F(app_test, append_trailing_slash)
{
    client->async_get("/blah").run();
    EXPECT_EQ(response.result(), beast::http::status::temporary_redirect);
    EXPECT_EQ(response.at(beast::http::field::location), "/blah/");
}

TEST_F(mock_app_test, domains_libvirt_error)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(nullptr));

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();

    EXPECT_EQ(response.result(), beast::http::status::internal_server_error);

    auto object = json::parse(response.body());
    EXPECT_TRUE(object.isObject());
    EXPECT_EQ(object["detail"], "Unable to connect to libvirt");
}

TEST_F(mock_app_test, domain_not_found)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(nullptr));

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);

    // When routes are matched but not_found is returned, there should
    // be an Allow header present with configured methods.
    EXPECT_EQ(response.at(beast::http::field::allow), "GET, OPTIONS");
}

TEST_F(mock_app_test, persistent_virt_connection)
{
    EXPECT_CALL(lv, virConnectOpen(_)).Times(2).WillRepeatedly(Return(conn));

    domain_ptr dom = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .Times(2)
        .WillRepeatedly(Return(dom));
    EXPECT_CALL(lv, virDomainGetAutostart(_, _))
        .Times(2)
        .WillRepeatedly(Invoke([](auto, int *autostart) {
            *autostart = 0;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetID(_)).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_))
        .Times(2)
        .WillRepeatedly(Return("test"));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .Times(2)
        .WillRepeatedly(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(4);
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).Times(2);

    client->on_response_override([this](const auto &response_) {
        response = response_;
    });

    testing::internal::CaptureStdout();

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();
    EXPECT_EQ(response.result(), beast::http::status::ok);

    // Doesn't do anything past here
    auto &connection = app_->pool().get(username);
    EXPECT_THROW(libvirt_close(nullptr, 0, &connection.closed()),
                 webvirt::retry_error);
    EXPECT_TRUE(connection.closed());

    EXPECT_NO_THROW(libvirt_free(nullptr));

    client->close();
    client->on_response_override([this](const auto &response_) {
        response = response_;
        io_.stop();
    });

    // Run get again after we've closed the libvirt connection.
    // During this run, reconnection will be made.
    client->async_get(endpoint.c_str()).run();
    EXPECT_EQ(response.result(), beast::http::status::ok);

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Reconnected to libvirt"), std::string::npos);
}

TEST_F(websocket_test, websocket)
{
    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _));

    std::string message("test");
    client->on_handshake([&](auto c, auto response) {
        std::stringstream ss;
        ss << response;
        logger::info(
            fmt::format("Handshake:\n{}-- End of handshake --", ss.str()));

        c->async_write(message);
    });
    client->on_write([](auto c) {
        c->close();
    });

    auto endpoint = fmt::format("/users/{}/websocket/", username);
    client->async_connect(endpoint).run();
}

TEST_F(websocket_test, error_on_read)
{
    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _));

    app_->server().on_error([this](const char *, beast::error_code) {
        io_.stop();
    });
    app_->server().on_handshake([this](websocket::connection_ptr) {
        client->shutdown(net::unix::socket::shutdown_both);
    });

    auto endpoint = fmt::format("/users/{}/websocket/", username);
    client->async_connect(endpoint).run();
}

TEST_F(websocket_test, error_on_accept)
{
    app_->server().on_error([this](const char *, beast::error_code) {
        io_.stop();
    });
    app_->server().on_websock_accept([](auto websock) {
        logger::info("on_websock_accept");
        websock->shutdown(net::unix::socket::shutdown_both);
    });

    auto endpoint = fmt::format("/users/{}/websocket/", username);
    client->on_error([this](const char *, beast::error_code) {
        client_io_.stop();
    });
    client->async_connect(endpoint).run();
}

TEST_F(websocket_test, connection_write)
{
    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _));

    app_->server().on_handshake([](websocket::connection_ptr conn) {
        conn->write("test");
    });

    auto endpoint = fmt::format("/users/{}/websocket/", username);
    std::string message;
    client->on_read([&message](auto client, const auto &str) {
        message = str;
        client->close();
    });
    client->async_connect(endpoint).run();

    EXPECT_EQ(message, "test");
}

TEST_F(websocket_test, error_on_write)
{
    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _));

    app_->server().on_handshake([](auto ws) {
        ws->shutdown(net::unix::socket::shutdown_send);
        ws->write("test");
    });

    auto endpoint = fmt::format("/users/{}/websocket/", username);
    client->on_error([this](const char *, beast::error_code) {
        client_io_.stop();
    });
    client->async_connect(endpoint).run();

    app_->server().on_error([this](const char *, auto) {
        io_.stop();
    });
    app_->server().on_handshake_override(
        http::noop<websocket::connection_ptr>());

    client = std::make_shared<websocket::client>(client_io_, socket_path);
    client->on_error([this](const char *, beast::error_code) {
        client_io_.stop();
    });
    client->on_handshake([](auto ws, auto) {
        ws->shutdown(net::unix::socket::shutdown_send);
        ws->async_write("test");
    });

    client->async_connect(endpoint).run();
}

TEST_F(websocket_test, events)
{
    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _));

    virt::lifecycle_callback cb(virt::lifecycle_event::on_event_handler);
    std::atomic<bool> ready = false;
    virt::connection *conn_ = nullptr;
    app_->on_virt_event_registration([&conn_, &ready](virt::connection &conn) {
        conn_ = &conn;
        ready = true;
    });

    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return("test"));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, auto, auto) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(2);

    webvirt::domain_ptr ptr_ = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virEventRunDefaultImpl())
        .WillRepeatedly(Invoke([this, &ready, &conn_, ptr_] {
            if (!ready)
                return 0;

            virt::event_function cb =
                virt::get_event_callback(VIR_DOMAIN_EVENT_ID_LIFECYCLE);
            virt::lifecycle_function f =
                reinterpret_cast<virt::lifecycle_function>(
                    reinterpret_cast<void *>(cb));

            auto &events = app_->events(conn_->user());
            auto &lifecycle_event = events.get(VIR_DOMAIN_EVENT_ID_LIFECYCLE);

            f(conn_->get_ptr().get(),
              ptr_.get(),
              VIR_DOMAIN_EVENT_SHUTDOWN,
              0,
              &lifecycle_event);

            ready = false;
            return 0;
        }));

    client->on_read([](auto client, auto text) {
        client->close();
        std::cout << text;
    });
    auto endpoint = fmt::format("/users/{}/websocket/", username);
    client->async_connect(endpoint).run();
}
