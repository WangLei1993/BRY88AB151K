#include "esp_log.h"

#include "influxdb.hpp"

namespace cubestone_wang 
{

namespace influxdb
{

const char *const Influxdb::LOG_TAG = "INFLUXDB";

Influxdb::Influxdb(const std::string &host,
                   const uint16_t &port,
                   const std::string &token,
                   const std::string &org,
                   const std::string &bucket,
                   const uint8_t &timeout)
{
    this->host = host;
    this->port = port;
    this->token = token;
    this->org = org;
    this->bucket = bucket;
    this->timeout = timeout;
}

bool Influxdb::WritePoint(const Point &point)
{
    bool result = false;
    std::string line = point.ToLineProtocol();
    std::string query = "bucket=" + this->bucket + "&org=" + this->org;
    std::string authorization = "Token "+this->token;
    esp_http_client_config_t config;
    memset((void *)&config, 0, sizeof(config));
    config.host = this->host.c_str();
    config.port = this->port;
    config.path = "/api/v2/write";
    config.transport_type = HTTP_TRANSPORT_OVER_TCP;
    config.timeout_ms = this->timeout * 1000;
    config.query = query.c_str();
    esp_http_client_handle_t client = esp_http_client_init(&config);
    ESP_ERROR_CHECK(esp_http_client_set_method(client, HTTP_METHOD_POST));
    ESP_ERROR_CHECK(esp_http_client_set_header(client, "Authorization", authorization.c_str()));
    ESP_ERROR_CHECK(esp_http_client_set_post_field(client, line.c_str(), strlen(line.c_str())));
    auto err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(Influxdb::LOG_TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    } else {
        auto status = esp_http_client_get_status_code(client);
        if (status != 204) {
            ESP_LOGE(Influxdb::LOG_TAG, "HTTP POST request failed: %d", status);
        } else {
            result = true;
        }
    } 
    esp_http_client_cleanup(client);
    return result;
}

}

}
