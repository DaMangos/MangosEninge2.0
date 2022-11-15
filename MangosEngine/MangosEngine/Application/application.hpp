#pragma once
#define GLFW_INCLUDE_VULKAN
#include "vulkan.hpp"
namespace mgo
{
#pragma mark - Application
    class Application final
    {
    private:
        glfw::Window window_;
        vk::Instance instance_;
        vk::DebugUtilsMessenger debugUtilsMessenger_;
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
