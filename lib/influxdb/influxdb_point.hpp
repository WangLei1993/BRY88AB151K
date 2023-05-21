#ifndef _influxdb_point_hpp_
#define _influxdb_point_hpp_

#include <string>

namespace cubestone_wang 
{

namespace influxdb
{

class Point
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        Point(const std::string &measurement);
        void AddTag(const std::string &name, const std::string &value);
        void AddField(const std::string &name, const std::string &value);
        void AddField(const std::string &name, const bool &value);
        void AddField(const std::string &name, const long long &value);
        void AddField(const std::string &name, const double &value, uint8_t decimal_places=2);
        void SetTimestamp(const time_t timestamp);
        std::string ToLineProtocol() const;
    private:
        std::string measurement;
        std::string tags;
        std::string fields;
        time_t timestamp;
        void putField(const std::string &name, const std::string &value);
};

}

}

#endif // _influxdb_point_hpp_
