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
#include "domains.hpp"
#include "../mocks/libvirt.hpp"
#include "../util/config.hpp"
#include "../util/json.hpp"
#include "../virt/connection.hpp"
#include "../virt/domain.hpp"
#include <gtest/gtest.h>
using namespace webvirt;
using namespace std::string_literals;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class domains_test : public Test
{
protected:
    mocks::libvirt lv;

    virt::connection conn_;
    http::request request_;
    http::response response_;

    views::domains views_;

public:
    void SetUp() override
    {
        libvirt::change(lv);
        libvirt::connect_ptr ptr = std::make_shared<webvirt::connect>();
        EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(ptr));
        EXPECT_CALL(lv, virConnectRegisterCloseCallback(_, _, _, _));
        conn_.connect("socket.sock");

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

        request_.method(beast::http::verb::get);
    }

    void TearDown() override
    {
        libvirt::reset();
    }

protected:
    std::string libvirt_domain_xml(
        unsigned id = 1, unsigned int vcpu = 2,
        unsigned int current_memory = 1024, unsigned int memory = 1024,
        std::vector<std::tuple<std::string, std::string, std::string,
                               std::string, std::string, std::string>>
            disks = {},
        std::vector<std::tuple<std::string, std::string, std::string>>
            interfaces = {})
    {
        std::string disks_;
        for (auto [device,
                   driver_name,
                   driver_type,
                   source_file,
                   target_dev,
                   target_bus] : disks) {
            disks_.append(fmt::format(R"(
<disk device="{}">
    <driver name="{}" type="{}" />
    <source file="{}" />
    <target dev="{}" bus="{}" />
</disk>
)",
                                      device,
                                      driver_name,
                                      driver_type,
                                      source_file,
                                      target_dev,
                                      target_bus));
        }

        std::string interfaces_;
        for (auto [mac, model, name] : interfaces) {
            interfaces_.append(fmt::format(R"(
<interface>
    <mac address="{}" />
    <model type="{}" />
    <alias name="{}" />
</interface>
)",
                                           mac,
                                           model,
                                           name));
        }

        return fmt::format(R"(
<domain id="{}">
    <vcpu>{}</vcpu>
    <currentMemory>{}</currentMemory>
    <memory>{}</memory>
    <os>
        <type arch="x86_64" machine="pc-q35-7.2">hvm</type>
    </os>
    <devices>
        {}
        {}
        <controller index="0" model="qemu-xhci" ports="15" type="usb">
            <address bus="0x02" domain="0x0000" function="0x0" slot="0x00" type="pci" />
        </controller>
        <controller index="1" model="qemu-xhci" ports="15" type="usb">
            <address bus="0x03" domain="0x0000" function="0x0" slot="0x00" type="pci" />
        </controller>
    </devices>
</domain>
)",
                           id,
                           vcpu,
                           current_memory,
                           memory,
                           disks_,
                           interfaces_);
    }

    std::smatch make_location(const std::string &pattern,
                              const std::string &uri)
    {
        std::regex re(pattern);
        std::smatch m;
        std::regex_match(uri, m, re);
        return m;
    }
};

TEST_F(domains_test, domains)
{
    std::vector<libvirt::domain_ptr> domains;
    libvirt::domain_ptr dom = std::make_shared<webvirt::domain>();
    domains.emplace_back(dom);
    EXPECT_CALL(lv, virConnectListAllDomains(_, _)).WillOnce(Return(domains));

    const char *domain_name = "test-domain";
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return(domain_name));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *reason, int) {
            *state = VIR_DOMAIN_RUNNING;
            *reason = 0;
            return 0;
        }));
    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));

    auto location = make_location(R"(^/users/([^/]+)/domains/([^/]+)/$)",
                                  "/users/test/domains/test/");
    libvirt::domain_ptr domain = std::make_shared<webvirt::domain>();
    views_.index(conn_, location, request_, response_);

    auto array = json::parse(response_.body());
    EXPECT_TRUE(array.isArray());
    EXPECT_EQ(array.size(), 1);

    auto object =
        array.get(Json::ArrayIndex(0), Json::Value(Json::objectValue));
    EXPECT_EQ(object["id"], 1);
    EXPECT_EQ(object["name"]["text"], "test-domain");
    EXPECT_EQ(object["state"]["attrib"]["id"], VIR_DOMAIN_RUNNING);
    EXPECT_EQ(object["state"]["attrib"]["string"], "Running");
}

TEST_F(domains_test, show)
{
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(2);
    EXPECT_CALL(lv, virDomainGetAutostart(_, _))
        .WillOnce(Invoke([](auto, int *autostart) {
            *autostart = 0;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return("test"));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    auto disk = std::make_tuple("disk"s,
                                "test_driver"s,
                                "sata"s,
                                "/path/to/source.qcow"s,
                                "vda"s,
                                "virtio"s);
    auto iface =
        std::make_tuple("aa:bb:cc:dd:11:22:33:44"s, "virtio"s, "net0"s);
    auto buffer = libvirt_domain_xml(1, 2, 1024, 1024, { disk }, { iface });
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(buffer));

    auto block_info_ptr = std::make_shared<webvirt::block_info>();
    EXPECT_CALL(lv, virDomainGetBlockInfo(_, _, _))
        .WillRepeatedly(Return(block_info_ptr));

    auto location = make_location(R"(^/users/([^/]+)/domains/([^/]+)/$)",
                                  "/users/test/domains/test/");
    libvirt::domain_ptr domain = std::make_shared<webvirt::domain>();
    views_.show(conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    auto data = json::parse(response_.body());
    const auto &disk_json = data["devices"]["disk"][0];
    const auto &block_info = disk_json["block_info"];
    EXPECT_EQ(block_info["unit"], "KiB");
    EXPECT_EQ(block_info["capacity"], 0);
    EXPECT_EQ(block_info["allocation"], 0);
    EXPECT_EQ(block_info["physical"], 0);
}

TEST_F(domains_test, show_cdrom)
{
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(2);
    EXPECT_CALL(lv, virDomainGetAutostart(_, _))
        .WillOnce(Invoke([](auto, int *autostart) {
            *autostart = 0;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return("test"));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    auto disk = std::make_tuple(
        "cdrom"s, "qemu"s, "raw"s, "/path/to/source.iso"s, "sda"s, "sata"s);
    auto iface =
        std::make_tuple("aa:bb:cc:dd:11:22:33:44"s, "virtio"s, "net0"s);
    auto buffer = libvirt_domain_xml(1, 2, 1024, 1024, { disk });
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(buffer));

    auto location = make_location(R"(^/users/([^/]+)/domains/([^/]+)/$)",
                                  "/users/test/domains/test/");
    libvirt::domain_ptr domain = std::make_shared<webvirt::domain>();
    views_.show(conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::ok);
}

TEST_F(domains_test, domain_start)
{
    EXPECT_CALL(lv, virDomainCreate(_)).WillOnce(Return(0));

    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *, int) {
            *state = 1;
            return 0;
        }));
    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return("test"));

    auto location = make_location(R"(^/users/([^/]+)/domains/([^/]+)/start/$)",
                                  "/users/test/domains/test/start/");
    auto domain = std::make_shared<webvirt::domain>();
    request_.method(boost::beast::http::verb::post);
    views_.start(conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::created);
}

TEST_F(domains_test, domain_start_error)
{
    EXPECT_CALL(lv, virDomainCreate(_)).WillOnce(Return(-1));

    auto location = make_location(R"(^/users/([^/]+)/domains/([^/]+)/start/$)",
                                  "/users/test/domains/test/start/");
    auto domain = std::make_shared<webvirt::domain>();
    request_.method(boost::beast::http::verb::post);
    views_.start(conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::bad_request);
}

TEST_F(domains_test, domain_shutdown)
{
    EXPECT_CALL(lv, virDomainShutdown(_)).WillOnce(Return(0));

    const char *domain_name = "test-domain";
    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return(domain_name));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(2);
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_SHUTDOWN;
            return 0;
        }))
        .WillRepeatedly(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_SHUTOFF;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/shutdown/$)",
                      "/users/test/domains/test/shutdown/");
    request_.method(boost::beast::http::verb::post);

    auto domain = std::make_shared<webvirt::domain>();
    views_.shutdown(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::ok);
}

TEST_F(domains_test, domain_shutdown_timeout)
{
    EXPECT_CALL(lv, virDomainShutdown(_)).WillOnce(Return(0));

    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillRepeatedly(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/shutdown/$)",
                      "/users/test/domains/test/shutdown/");
    auto domain = std::make_shared<webvirt::domain>();
    request_.method(boost::beast::http::verb::post);
    views_.shutdown(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::gateway_timeout);
}

TEST_F(domains_test, domain_shutoff_timeout)
{
    EXPECT_CALL(lv, virDomainShutdown(_)).WillOnce(Return(0));

    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillRepeatedly(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_SHUTDOWN;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/shutdown/$)",
                      "/users/test/domains/test/shutdown/");
    auto domain = std::make_shared<webvirt::domain>();
    request_.method(boost::beast::http::verb::post);
    views_.shutdown(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::gateway_timeout);
}

TEST_F(domains_test, domain_shutdown_bad_request)
{
    EXPECT_CALL(lv, virDomainShutdown(_)).WillOnce(Return(-1));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/shutdown/$)",
                      "/users/test/domains/test/shutdown/");
    request_.method(boost::beast::http::verb::post);

    auto domain = std::make_shared<webvirt::domain>();
    views_.shutdown(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::bad_request);
}

TEST_F(domains_test, autostart_post)
{
    int autostart = 0;
    EXPECT_CALL(lv, virDomainSetAutostart(_, _))
        .WillOnce(Invoke([&](auto, int autostart_) {
            autostart = autostart_;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
                      "/users/test/domains/test/autostart/");
    request_.method(boost::beast::http::verb::post);

    libvirt::domain_ptr domain = std::make_shared<webvirt::domain>();
    views_.autostart(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), boost::beast::http::status::ok);
    EXPECT_EQ(autostart, 1);
}

TEST_F(domains_test, autostart_delete)
{
    int autostart = 1;
    EXPECT_CALL(lv, virDomainSetAutostart(_, _))
        .WillOnce(Invoke([&](auto, int autostart_) {
            autostart = autostart_;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
                      "/users/test/domains/test/autostart/");
    request_.method(boost::beast::http::verb::delete_);

    libvirt::domain_ptr domain = std::make_shared<webvirt::domain>();
    views_.autostart(
        conn_, virt::domain(domain), location, request_, response_);

    EXPECT_EQ(response_.result(), boost::beast::http::status::ok);
    EXPECT_EQ(autostart, 0);
}

TEST_F(domains_test, metadata_bad_request)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    request_.method(boost::beast::http::verb::post);

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::bad_request);
}

TEST_F(domains_test, metadata_title)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    EXPECT_CALL(lv, virDomainSetMetadata(_, _, _, _, _, _))
        .WillOnce(Return(0));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _))
        .WillOnce(Return(""))
        .WillRepeatedly(Return("Test Title"));

    request_.method(boost::beast::http::verb::post);

    Json::Value data(Json::objectValue);
    data["title"] = "Test Title";
    boost::beast::ostream(request_.body()) << json::stringify(data);
    request_.content_length(request_.body().size());

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    auto domain = conn_.domain("test");
    auto title = domain.metadata(VIR_DOMAIN_METADATA_TITLE, nullptr, 0);
    EXPECT_EQ(title, "Test Title");

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    views_.metadata(
        conn_, virt::domain(domain_ptr), location, request_, response_);
    EXPECT_EQ(response_.result(), beast::http::status::not_modified);
}

TEST_F(domains_test, metadata_description)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    EXPECT_CALL(lv, virDomainSetMetadata(_, _, _, _, _, _))
        .WillOnce(Return(0));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _))
        .WillOnce(Return(""))
        .WillOnce(Return(""))
        .WillRepeatedly(Return("Test description."));

    request_.method(boost::beast::http::verb::post);

    Json::Value data(Json::objectValue);
    data["description"] = "Test description.";
    boost::beast::ostream(request_.body()) << json::stringify(data);
    request_.content_length(request_.body().size());

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    auto domain = conn_.domain("test");
    auto title = domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION, nullptr, 0);
    EXPECT_EQ(title, "Test description.");

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    views_.metadata(conn_, domain, location, request_, response_);
    EXPECT_EQ(response_.result(), beast::http::status::not_modified);
}

TEST_F(domains_test, bootmenu_post)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    auto xml = R"(
<domain>
    <os>
        <type arch="x86_64" machine="pc-q35-7.2" />
        <boot dev="hd" />
    </os>
</domain>
)";
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(xml));
    EXPECT_CALL(lv, virDomainDefineXML(_, _)).WillOnce(Return(domain_ptr));

    request_.method(boost::beast::http::verb::post);
    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/bootmenu/$)",
                      "/users/test/domains/test/bootmenu/");
    views_.bootmenu(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    auto data = json::parse(response_.body());
    EXPECT_EQ(data["bootmenu"]["attrib"]["enable"].asString(), "yes");
}

TEST_F(domains_test, bootmenu_delete)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    auto xml = R"(
<domain>
    <os>
        <type arch="x86_64" machine="pc-q35-7.2" />
        <boot dev="hd" />
    </os>
</domain>
)";
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(xml));
    EXPECT_CALL(lv, virDomainDefineXML(_, _)).WillOnce(Return(domain_ptr));

    request_.method(boost::beast::http::verb::delete_);
    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/bootmenu/$)",
                      "/users/test/domains/test/bootmenu/");
    views_.bootmenu(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    auto data = json::parse(response_.body());
    EXPECT_EQ(data["bootmenu"]["enable"].asBool(), false);
}

TEST_F(domains_test, bootmenu_define_xml_error)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(""));
    EXPECT_CALL(lv, virDomainDefineXML(_, _)).WillOnce(Return(nullptr));

    request_.method(boost::beast::http::verb::post);
    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/bootmenu/$)",
                      "/users/test/domains/test/bootmenu/");
    views_.bootmenu(
        conn_, virt::domain(domain_ptr), location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::internal_server_error);

    auto data = json::parse(response_.body());
    EXPECT_EQ(data["detail"].asString(), "Unable to replace domain XML");
}
