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
#include <algorithm>
#include <limits>

#include <cstdint>
#include <cstring>
#include <cmath>

namespace mgo
{
#pragma mark - App
    class App final
    {
    private:
#pragma mark - App::RenderWindow
        class RenderWindow final
        {
        private:
            friend App;
            
            const std::string       windowName_;
            const std::uint32_t     windowHight_;
            const std::uint32_t     windowWidth_;
            GLFWwindow*             pWindow_;
            
            RenderWindow(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight);
            
            ~RenderWindow() noexcept;
            
            VkResult createSurface(VkInstance instance, VkSurfaceKHR* pSurface) const;
            
            VkExtent2D GetFramebufferSize() const noexcept;
            
            bool shouldClose() const noexcept;
            
            static void pollEvents() noexcept;
            
            static void errorCallback(int error, const char* description);
        };
        
#pragma mark - App::Device
        class Device final
        {
        private:
            friend App;
            
            struct QueueFamilyIndices
            {
                std::optional<std::uint32_t>    graphicsFamily_;
                std::optional<std::uint32_t>    presentFamily_;
                float                           queuePriority_;
            };
            
            std::string                     appName_;
            std::string                     engineName_;
            const std::vector<const char*>  validationLayers_;
            const std::vector<const char*>  instanceExtensions_;
            const std::vector<const char*>  deviceExtensions_;
            QueueFamilyIndices              indices_;
            VkInstance                      instance_;
            VkDebugUtilsMessengerEXT        debugMessenger_;
            VkSurfaceKHR                    surface_;
            VkPhysicalDevice                physicalDevice_;
            VkDevice                        device_;
            VkQueue                         graphicsQueue_;
            VkQueue                         presentQueue_;
            VkSwapchainKHR                  swapchain_;
            
            Device(const std::string& appName, const RenderWindow& window);
            
            ~Device();
            
            std::vector<const char*> getValidationLayers() noexcept;

            std::vector<const char*> getinstanceExtensions() noexcept;
            
            std::vector<const char*> getdeviceExtensions() noexcept;
                        
            void createVulkanInstance();
            
            void createDebugMessenger();

            void createSurface(const RenderWindow& window);

            void createDevice();
            
            void createSwapchain(const RenderWindow& window);
            
            void destoryVulkanInstance() noexcept;
            
            void destoryDebugMessenger() noexcept;
            
            void destroySurface() noexcept;

            void destroyDevice() noexcept;
            
            void destroySwapchain() noexcept;
            
            std::uint8_t rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept;
                        
            void populateApplicationInfo(VkApplicationInfo& appInfo) const noexcept;
            
            void populateDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& pDebugCreateInfo) const noexcept;
            
            void populateInstanceCreateInfo(VkInstanceCreateInfo& instanceCreateInfo,
                                            const VkApplicationInfo* pAppnInfo,
                                            const VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const noexcept;
            
            void populateDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo, std::uint32_t queueFamily) const noexcept;
            
            void populatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& physicalDeviceFeatures) const noexcept;
            
            void populateDeviceCreateInfo(VkDeviceCreateInfo& deviceCreateInfo,
                                          const std::vector<VkDeviceQueueCreateInfo>& QueueCreateInfos,
                                          const VkPhysicalDeviceFeatures* pPhysicalDeviceFeatures) const noexcept;
            
            void populateQueueFamilyIndices(QueueFamilyIndices& indices, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, float queuePriority) const noexcept;
                        
            void populateSwapchainCreateInfoKHR(VkSwapchainCreateInfoKHR& SwapchainCreateInfo, const RenderWindow& window) const noexcept;
            
            bool checkValidationLayerSupport() const noexcept;

            bool checkInstanceExtensionSupport() const noexcept;
            
            bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const noexcept;
            
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
            
            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
            {
                for (const auto& availableFormat : availableFormats)
                    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                        return availableFormat;
                
                return availableFormats[0];
            }
            
            VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
                return VK_PRESENT_MODE_FIFO_KHR;
            }
        };
        
#pragma mark - App::Pipeline
        class Pipeline final
        {
        private:
            friend App;
            
            Pipeline();
            
            std::vector<char> readFile(const std::string& filePath) const;
            
            void createPipeline(const std::string& fragFilePath, const std::string& vertFilePath);
        };
        
        std::string appName_;
        RenderWindow window_;
        Device device_;
        Pipeline pipeline_;
    public:
        App();
        
        ~App();
                
        void run();
    };
}
