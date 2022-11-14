#pragma once
#include "glfw.hpp"
#include <vulkan/vulkan.h>
#include <map>
#include <set>

namespace vk
{
#pragma mark - vk::Instance
    class Instance final
    {
    private:
        VkInstance instance_;
        const std::string engineName_;
        const std::string applicationName_;
        const std::vector<const char*> extensions_;
        
    public:
        Instance(const std::string& engineName, const std::string& applicationName, const glfw::Window& window);
        
        ~Instance() noexcept;
        
        const VkInstance get() const noexcept;
        
    private:
        void checkInstanceExtensionSupport() const noexcept;

        std::vector<const char*> getExtensions(const glfw::Window& window) const noexcept;
        
        VkApplicationInfo getVkApplicationInfo() const noexcept;
        
        VkDebugUtilsMessengerCreateInfoEXT getVkDebugUtilsMessengerCreateInfoEXT() const noexcept;
    };
    
#pragma mark - vk::DebugUtilsMessenger
    class DebugUtilsMessenger final
    {
    private:
        VkDebugUtilsMessengerEXT debugUtilsMessengerEXT_;
        const Instance& instance_;
        
    public:
        DebugUtilsMessenger(const Instance& instance);
        
        ~DebugUtilsMessenger() noexcept;

        const VkDebugUtilsMessengerEXT get() const noexcept;
        
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void* pUserData) noexcept;
        
    private:
        VkDebugUtilsMessengerCreateInfoEXT getVkDebugUtilsMessengerCreateInfoEXT() const noexcept;
        
        static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks* pAllocator,
                                                     VkDebugUtilsMessengerEXT* pDebugMessenger) noexcept;
        
        
        static void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                                  VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator) noexcept;
    };
#pragma mark - vk::Surface
    class Surface final
    {
    private:
        VkSurfaceKHR surface_;
        const Instance& instance_;

    public:
        Surface(const Instance& instance, const glfw::Window& window);
        
        ~Surface() noexcept;
        
        const VkSurfaceKHR get() const noexcept;
    };

    
    
#pragma mark - vk::Device
    class Device final
    {
    private:
        struct QueueFamilyIndices
        {
            std::optional<std::uint32_t> graphicsFamily_;
            std::optional<std::uint32_t> presentFamily_;
            float queuePriority_;
        };
        
        VkDevice device_;
        VkPhysicalDevice physicalDevice_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;
        QueueFamilyIndices queueFamilyIndices_;
        const Instance& instance_;
        const Surface& surface_;
        const std::vector<const char*>  extensions_;
        
    public:
        Device(const Instance& instance, const Surface& surface);
        
        ~Device() noexcept;
        
        const VkDevice get() const noexcept;
        
    private:
        std::vector<const char*> getExtensions() const noexcept;

        std::uint8_t rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept;
                
        bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, bool logResults) const noexcept;
        
        QueueFamilyIndices getQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                                 VkSurfaceKHR surface,
                                                 float queuePriority) const noexcept;
        
        void pickPhysicalDevice();
        
        VkDeviceQueueCreateInfo getDeviceQueueCreateInfo(std::uint32_t queueFamily) const noexcept;
        
        VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() const noexcept;
    };

}
