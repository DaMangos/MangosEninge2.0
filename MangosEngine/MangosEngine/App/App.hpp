#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>
#include <tuple>
#include <exception>
#include <optional>

#include <cstring>
#include <cmath>

namespace mgo
{
#pragma mark - App
    class App final
    {
    public:
        
#pragma mark - App::RenderWindow
        class RenderWindow final
        {
        private:
            friend App;
            
            const std::string       windowName;
            const std::uint32_t     windowHight;
            const std::uint32_t     windowWidth;
            GLFWwindow*             pWindow;

            RenderWindow(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight);
            
            ~RenderWindow() noexcept;
            
            bool shouldClose() const noexcept;
            
            static void pollEvents() noexcept;
            
            static void errorCallback(int error, const char* description);
        };
        
#pragma mark - App::Device
        class Device final
        {
        private:
            friend App;
            
            class QueueFamilyIndices
            {
            private:
                friend Device;
                
                std::optional<std::uint32_t> graphicsFamily;
                
                QueueFamilyIndices() = default;
                
                QueueFamilyIndices(VkPhysicalDevice device) noexcept;
                                
                void findQueueFamilies(VkPhysicalDevice device) noexcept;

                bool isComplete() const noexcept;
            };
            
            std::string                     appName;
            std::string                     engineName;
            const std::vector<const char*>  requiredExtensions;
            const std::vector<const char*>  validationLayers;
            QueueFamilyIndices              indices;
            VkInstance                      instance;
            VkDebugUtilsMessengerEXT        debugMessenger;
            VkPhysicalDevice                physicalDevice;
            VkDevice                        device;
            VkQueue                         graphicsQueue;

            Device(const std::string& appName);
            
            ~Device();
            
            std::vector<const char*> getRequiredExtensions() noexcept;
            
            std::vector<const char*> getValidationLayers() noexcept;
            
            void setupVulkanInstance();
            
            void setupDebugMessenger();
            
            void setupPhysicalDevice();
            
            void setupDevice();

            void destoryVulkanInstance() noexcept;
            
            void destoryDebugMessenger() noexcept;
            
            void destroyDevice() noexcept;
            
            void populateApplicationInfo(VkApplicationInfo& applicationInfo) const noexcept;
            
            void populateDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& pDebugCreateInfo) const noexcept;
            
            void populateInstanceCreateInfo(VkInstanceCreateInfo& instanceCreateInfo,
                                            VkApplicationInfo* pApplicationInfo,
                                            VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const noexcept;
            
            void populateDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo) const noexcept;
            
            void populatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& physicalDeviceFeatures) const noexcept;
            
            void populateDeviceCreateInfo(VkDeviceCreateInfo& deviceCreateInfo,
                                          VkDeviceQueueCreateInfo* queueCreateInfo,
                                          VkPhysicalDeviceFeatures* physicalDeviceFeatures) const noexcept;

            void checkExtensionSupport() const noexcept;

            void checkValidationLayerSupport() const noexcept;
            
            static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                         const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugUtilsMessengerEXT* pDebugMessenger) noexcept;

            static void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                      VkDebugUtilsMessengerEXT debugMessenger,
                                                      const VkAllocationCallbacks* pAllocator) noexcept;
            
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData) noexcept;
        };
        
        std::string appName;
        RenderWindow window;
        Device device;
                        
    public:
        App();
        
        ~App();
                
        void run();
    };
}
