#ifndef _mdns_hpp_
#define _mdns_hpp_

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mdns.h"

namespace cubestone_wang 
{

namespace mdns
{

class MDNS
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        struct TxtItem {
            std::string Key;
            std::string Value;
        };
        struct Service{
            std::string InstanceName;
            std::string Type;
            std::string Proto;
            uint16_t Port;
            std::vector<TxtItem> Txt;
        };
        static bool Start(const std::string &hostname, 
                          const std::string &instance_name="", 
                          const std::vector<Service> &services=std::vector<Service>());
        static void Stop();
    private:
        static bool start_flag;
        static SemaphoreHandle_t mutex;
};

}

}

#endif // _mdns_hpp_
