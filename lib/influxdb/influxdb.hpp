#ifndef _influxdb_hpp_
#define _influxdb_hpp_

#include <string>

#include "esp_http_client.h"

#include "influxdb_point.hpp"

namespace cubestone_wang 
{

namespace influxdb
{

class Influxdb
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        Influxdb(const std::string &host,
                 const uint16_t &port,
                 const std::string &token,
                 const std::string &org,
                 const std::string &bucket,
                 const uint8_t &timeout=5);
        bool WritePoint(const Point &point);
    private:
        std::string host;
        std::uint16_t port;
        std::string token;
        std::string org;
        std::string bucket;
        uint8_t timeout;
};

}

}

#endif // _influxdb_hpp_
