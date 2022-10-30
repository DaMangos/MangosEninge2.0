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

#include <cstring>
#include <cmath>
#include <assert.h>
 
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
        };
        
#pragma mark - App::Device
        class Device final
        {
        public:
            friend App;
            
            std::string                     appName;
            std::string                     engineName;
            const std::vector<const char*>  requiredExtensions;
            const std::vector<const char*>  validationLayers;
            VkInstance                      instance;
            VkDebugUtilsMessengerEXT        debugMessenger;
            VkPhysicalDevice                physicalDevice;
            VkDevice                        device;
            
            Device(const std::string& appName);
            
            ~Device();
            
            std::vector<const char*> getRequiredExtensions() noexcept;
            
            std::vector<const char*> getValidationLayers() noexcept;
            
            void setupVulkanInstance();
            
            void setupDebugMessenger();
            
            void destoryVulkanInstance() noexcept;
            
            void destoryDebugMessenger() noexcept;
            
            void populateApplicationInfo(VkApplicationInfo& applicationInfo) const noexcept;
            
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& pDebugCreateInfo) const noexcept;
            
            void populateInstanceCreateInfo(VkInstanceCreateInfo& instanceCreateInfo,
                                            VkApplicationInfo* pApplicationInfo,
                                            VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const noexcept;

            bool checkExtensionSupport() const noexcept;

            bool checkValidationLayerSupport() const noexcept;
            
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
                        
    public:
        App();
        
        ~App();
                
        void run();
    };
}
