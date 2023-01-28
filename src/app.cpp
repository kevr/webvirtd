#include "app.hpp"
using namespace webvirt;

app::app(webvirt::io_service &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    router_.route(R"(^/domains/?$)",
                  http::router::with_methods(
                      { beast::http::verb::post },
                      http::router::with_user(bind_user(&app::domains))));
    router_.route(R"(^/domains/([^/]+)/?$)",
                  http::router::with_methods(
                      { beast::http::verb::post },
                      http::router::with_user(bind_user(&app::domain))));

    server_.on_request([this](auto &, const auto &request, auto &response) {
        return router_.run(request, response);
    });
}

std::size_t app::run()
{
    return server_.run();
}

void app::domains(const std::string &user, const std::smatch &,
                  const beast::http::request<beast::http::dynamic_body> &,
                  beast::http::response<beast::http::string_body> &response)
{
    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    Json::Value json(Json::arrayValue);
    virt::connection conn;
    try {
        conn.connect(virt::uri(user));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what();
        json = Json::Value(Json::objectValue);
        json["detail"] = "Unable to connect to libvirt";
        response.result(beast::http::status::internal_server_error);
        auto output = json::stringify(json);
        response.body().append(output);
        response.content_length(response.body().size());
        return;
    }

    auto domains_ = conn.domains();
    for (auto &domain : domains_) {
        Json::Value map(Json::objectValue);
        for (auto &kv : domain) {
            map[kv.first] = kv.second;
        }
        json.append(map);
    }

    auto output = json::stringify(json);
    response.body().append(output);
    response.content_length(response.body().size());
}

void app::domain(const std::string &user, const std::smatch &location,
                 const beast::http::request<beast::http::dynamic_body> &,
                 beast::http::response<beast::http::string_body> &response)
{
    const std::string name(location[1]);

    // Any response we send is JSON-serialized
    response.set("Content-Type", "application/json");

    Json::Value json(Json::objectValue);
    virt::connection conn;
    try {
        conn.connect(virt::uri(user));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what();
        json = Json::Value(Json::objectValue);
        json["detail"] = "Unable to connect to libvirt";
        response.result(beast::http::status::internal_server_error);
        auto output = json::stringify(json);
        response.body().append(output);
        response.content_length(response.body().size());
        return;
    }

    auto domain = conn.domain(name);
    if (domain.find("id") == domain.end()) {
        json["detail"] = "Unable to find domain";
        response.result(beast::http::status::not_found);
    }

    for (auto &kv : domain) {
        json[kv.first] = kv.second;
    }

    auto output = json::stringify(json);
    response.body().append(output);
    response.content_length(response.body().size());
}
