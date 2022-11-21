#include "mgo_application.hpp"

namespace mgo
{
    Application::Application()
    :
    window_("Mangos Eninge", 500, 500),
    instance_("Mangos Enigne", "Mangos App", this->window_),
#ifdef DEBUG
    debugUtilsMessenger_(this->instance_),
#endif
    surface_(this->instance_, this->window_),
    physicalDevice_(this->instance_, this->surface_),
    device_(this->instance_, this->surface_, this->physicalDevice_),
    swapchain_(this->surface_, this->physicalDevice_, this->device_),
    imageViews_(this->surface_, this->physicalDevice_, this->device_, this->swapchain_),
    renderPass_(this->surface_, this->physicalDevice_, this->device_),
    pipelineLayout_(this->device_),
    pipeline_(this->device_, this->renderPass_, this->pipelineLayout_)
    {
        MGO_DEBUG_LOG_MESSAGE("Created mgo::Application!");
    }
    
    Application::~Application()
    {
        MGO_DEBUG_LOG_MESSAGE("Destroyed mgo::Application!");
    }
            
    void Application::run()
    {
        MGO_DEBUG_LOG_MESSAGE("Running mgo::Application");
        while (!this->window_.shouldClose())
        {
            this->window_.pollEvents();
        }
    }
}
