#include "esp_log.h"

#include "system.hpp"

#include "influxdb.hpp"

namespace cubestone_wang 
{

namespace influxdb
{

const char *const Point::LOG_TAG = "INFLUXDB_POINT";

Point::Point(const std::string &measurement)
{
    this->measurement = measurement;
    this->tags = "";
    this->fields = "";
    this->timestamp = 0;
}

void Point::AddTag(const std::string &name, const std::string &value)
{
    if (this->tags.length() > 0) {
        this->tags += ',';
    }
    this->tags += name;
    this->tags += '=';
    this->tags += value;
}

void Point::AddField(const std::string &name, const std::string &value)
{
    this->putField(name, "\""+value+"\"");
}

void Point::AddField(const std::string &name, const bool &value)
{
    if (value) {
        this->putField(name, "true");
    } else {
        this->putField(name, "false"); 
    }
}

void Point::AddField(const std::string &name, const long long &value)
{
    char buffer[32];
    memset((void *)buffer, 0, sizeof(buffer));
    sprintf(buffer, "%lldi", value);
    this->putField(name, buffer);
}

void Point::AddField(const std::string &name, const double &value, uint8_t decimal_places)
{
    char buffer[64];
    char format[16];
    memset((void *)buffer, 0, sizeof(buffer));
    memset((void *)format, 0, sizeof(format));
    sprintf(format, "%%.%ulf", decimal_places);
    sprintf(buffer, format, value);
    this->putField(name, std::string(buffer));
}

void Point::SetTimestamp(const time_t timestamp)
{
    this->timestamp = timestamp;
}

void Point::putField(const std::string &name, const std::string &value)
{
    if (this->fields.length() > 0) {
        this->fields += ',';
    }
    this->fields += name;
    this->fields += '=';
    this->fields += value;
}

std::string Point::ToLineProtocol() const
{
    std::string line;
    char buffer[32];
    memset((void *)buffer, 0, sizeof(buffer));
    if (this->timestamp == 0) {
        sprintf(buffer, "%lld", system::System::GetCurrentTimestamp()*1000*1000*1000);
    } else {
        sprintf(buffer, "%lld", this->timestamp*1000*1000*1000);
    }
    std::string timestamp = std::string(buffer);
    line.reserve(this->measurement.length() 
                 + 1
                 + this->tags.length()
                 + 1
                 + this->fields.length()
                 + 1
                 + timestamp.length());
    line += this->measurement;
    if (this->tags.length() > 0) {
        line += ",";
        line += this->tags;
    }
    line += " ";
    line += this->fields;
    line += " ";
    line += timestamp;
    return line;
}

}

}
