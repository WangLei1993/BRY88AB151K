#ifndef _ntp_hpp_
#define _ntp_hpp_

#include <string>
#include <vector>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace ntp 
{

class NTP
{
    public:
        // 日志标签
        static const char *const LOG_TAG;
        /**
         * @brief 启动
         *
         * @param wait_time 等待时间（秒）
         */
        static bool Start(const std::vector<std::string> &server_name_list, const uint32_t wait_time=30);
        /**
         * @brief 停止
         */
        static void Stop();
    private:
        static bool start_flag;
        static SemaphoreHandle_t mutex;
        static std::vector<std::string *> server_name_list;
};

}

}

#endif // _ntp_hpp_
