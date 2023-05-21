#include "application.hpp"

extern "C"
{
    void app_main(void)
    {   
        cubestone_wang::application::Application::Start();
    }
}
