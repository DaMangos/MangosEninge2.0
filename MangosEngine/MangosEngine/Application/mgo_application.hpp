#pragma once
#define GLFW_INCLUDE_VULKAN
#include "mgo_vulkan.hpp"
namespace mgo
{
#pragma mark - Application
    class Application final
    {
    private:
        glfw::Window window_;
        vk::Instance instance_;
#ifdef DEBUG
        vk::DebugUtilsMessenger debugUtilsMessenger_;
#endif
        vk::Surface surface_;
        vk::PhysicalDevice physicalDevice_;
        vk::Device device_;
        vk::Swapchain swapchain_;
        vk::ImageViews imageViews_;
        vk::RenderPass renderPass_;
        vk::PipelineLayout pipelineLayout_;
        vk::Pipeline pipeline_;
        
    public:
        Application();
        
        ~Application();
                
        void run();
    };
}
