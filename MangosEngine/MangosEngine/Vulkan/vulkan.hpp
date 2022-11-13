#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <string>
#include <vector>
#include <exception>


namespace vk
{
    const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
    
#pragma mark - vk::debugCallback
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData) noexcept;
    
#pragma mark - vk::Instance
    class Instance final
    {
    private:
        VkInstance                      instance_;
        const std::string               engineName_;
        const std::string               applicationName_;
        const std::vector<const char*>  extensions_;
        
    public:
        Instance(const std::string& engineName, const std::string& applicationName);
        
        ~Instance();
        
        const VkInstance get() const noexcept;
        
    private:
        
        VkApplicationInfo getVkApplicationInfo() const noexcept;
        
        VkDebugUtilsMessengerCreateInfoEXT getVkDebugUtilsMessengerCreateInfoEXT() const noexcept;
        
        VkInstanceCreateInfo getVkInstanceCreateInfo() const noexcept;
    };
    
#pragma mark - vk::DebugUtilsMessenger
    class DebugUtilsMessenger final
    {
    private:
        VkDebugUtilsMessengerEXT    debugUtilsMessengerEXT_;
        const Instance&             instance_;
        
    public:
        DebugUtilsMessenger(const Instance& instance);
        
        ~DebugUtilsMessenger();

        const VkDebugUtilsMessengerEXT get() const noexcept;
        
    private:
        
        VkDebugUtilsMessengerCreateInfoEXT getVkDebugUtilsMessengerCreateInfoEXT() const noexcept;
    };
}
