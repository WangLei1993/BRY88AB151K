#ifndef _scheduled_restart_hpp_
#define _scheduled_restart_hpp_

#include <string>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace cubestone_wang 
{

namespace scheduled_restart
{

class ScheduledRestart
{
    public:
        enum class DayType{
            RESTART_DAY_TYPE_SUNDAY  = 0,
            RESTART_DAY_TYPE_MONDAY  = 1,
            RESTART_DAY_TYPE_TUESDAY  = 2,
            RESTART_DAY_TYPE_WEDENSDAY  = 3,
            RESTART_DAY_TYPE_THURSDAY  = 4,
            RESTART_DAY_TYPE_FRIDAY  = 5,
            RESTART_DAY_TYPE_SATURDAY  = 6,
            RESTART_DAY_TYPE_EVERY_DAY = 7,
        };
        // 日志标签
        static const char *const LOG_TAG;
        static bool Start(const DayType day_type, const uint8_t hour, const uint8_t minute);
        static void Stop();
    private:
        static bool start_flag;
        static SemaphoreHandle_t mutex;
        static const std::string name;
};

}

}

#endif // _scheduled_restart_hpp_
