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
            
            VkSurfaceCapabilitiesKHR        surfaceCapabilities_;
            VkSurfaceFormatKHR              surfaceFormat_;
            VkPresentModeKHR                presentMode_;
            VkExtent2D                      extent_;
            VkSwapchainKHR                  swapchain_;
            std::vector<VkImage>            images_;
            std::vector<VkImageView>        imageViews_;
            VkRenderPass                    renderPass_;
            VkPipelineLayout                pipelineLayout_;
            VkPipeline                      pipeline_;


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
            
            void createImageViews();
            
            void createGraphicsPipeline();
            
            void createRenderPass();

            void destoryVulkanInstance() noexcept;
            
            void destoryDebugMessenger() noexcept;
            
            void destroySurface() noexcept;

            void destroyDevice() noexcept;
            
            void destroySwapchain() noexcept;
            
            void destroyImageViews() noexcept;
            
            void destroyRenderPass() noexcept;

            void destroyGraphicsPipeline() noexcept;
            
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
                        
            void populateSwapchainCreateInfoKHR(VkSwapchainCreateInfoKHR& SwapchainCreateInfo) const noexcept;
            
            void populateImageViewCreateInfo(VkImageViewCreateInfo& imageViewCreateInfo, VkImage image) const noexcept;
            
            void populateShaderModuleCreateInfo(VkShaderModuleCreateInfo& shaderModuleCreateInfo, const std::vector<char>& code) const noexcept;
            
            void populatePipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& pipelineShaderStageCreateInfo,
                                                       VkShaderStageFlagBits shaderStageFlagBits,
                                                       VkShaderModule shaderModule) const noexcept;
            
            void populatePipelineDynamicStateCreateInfo(VkPipelineDynamicStateCreateInfo& pipelineDynamicStateCreateInfo,
                                                                     const std::vector<VkDynamicState>& dynamicStates) const noexcept;
            
            void populatePipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& pipelineVertexInputStateCreateInfo) const noexcept;
            
            void populatePipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& pipelineInputAssemblyStateCreateInfo) const noexcept;
            
            void populateViewport(VkViewport& viewport)const noexcept;

            void populateRect2D(VkRect2D& rect2D) const noexcept;
                        
            void populatePipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& pipelineViewportStateCreateInfo,
                                                         const VkViewport* pViewport, const VkRect2D* pScissor) const noexcept;
            
            void populatePipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& pipelineRasterizationStateCreateInfo) const noexcept;
            
            void populatePipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo& pipelineMultisampleStateCreateInfo) const noexcept;
            
            void populatePipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateInfo& pipelineDepthStencilStateCreateInfo) const noexcept;
            
            void populatePipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& pipelineColorBlendAttachmentState) const noexcept;
            
            void populatePipelineColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo& pipelineColorBlendStateCreateInfo,
                                                           const VkPipelineColorBlendAttachmentState* pPipelineColorBlendAttachmentState) const noexcept;

            void populatePipelineLayoutCreateInfo(VkPipelineLayoutCreateInfo& pipelineLayoutCreateInfo) const noexcept;
            
            void populateAttachmentDescription(VkAttachmentDescription& attachmentDescription) const noexcept;
            
            void populateAttachmentReference(VkAttachmentReference& attachmentReference) const noexcept;
            
            void populateSubpassDescription(VkSubpassDescription& subpassDescription,
                                            const VkAttachmentReference* pColorAttachmentReference) const noexcept;

            void populateRenderPassCreateInfo(VkRenderPassCreateInfo& renderPassCreateInfo,
                                                           const std::vector<VkAttachmentDescription>& attachmentDescriptions,
                                                           const std::vector<VkSubpassDescription>& subpassDescriptions) const noexcept;
            
            void populateGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
                                                    const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfos,
                                                    const VkPipelineVertexInputStateCreateInfo* pPipelineVertexInputStateCreateInfo,
                                                    const VkPipelineInputAssemblyStateCreateInfo* pPipelineInputAssemblyStateCreateInfo,
                                                    const VkPipelineTessellationStateCreateInfo* pPipelineTessellationStateCreateInfo,
                                                    const VkPipelineViewportStateCreateInfo* pPipelineViewportStateCreateInfo,
                                                    const VkPipelineRasterizationStateCreateInfo* pPipelineRasterizationStateCreateInf,
                                                    const VkPipelineMultisampleStateCreateInfo* pPipelineMultisampleStateCreateInfo,
                                                    const VkPipelineDepthStencilStateCreateInfo* pPipelineDepthStencilStateCreateInfo,
                                                    const VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo,
                                                    const VkPipelineDynamicStateCreateInfo* pPipelineDynamicStateCreateInfo) const noexcept;
            
            bool checkValidationLayerSupport() const noexcept;

            bool checkInstanceExtensionSupport() const noexcept;
            
            bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const noexcept;
            
            static VkResult createDebugUtilsMessengerEXT(VkInstance instance,
                                                         const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                         const VkAllocationCallbacks* pAllocator,
                                                         VkDebugUtilsMessengerEXT* pDebugMessenger) noexcept;

            static void destroyDebugUtilsMessengerEXT(VkInstance instance,
                                                      VkDebugUtilsMessengerEXT debugMessenger,
                                                      const VkAllocationCallbacks* pAllocator) noexcept;
            
            static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData) noexcept;
            
            void setupSurfaceCapabilities() noexcept;

            void setupSurfaceFormat() noexcept;
            
            void setupPresentMode() noexcept;
            
            void setupExtent(const RenderWindow& window) noexcept;
            
            std::vector<char> readFile(const std::string& filePath) const;
            
            VkShaderModule createShaderModule(const std::vector<char>& code);
        };
    
        
        std::string appName_;
        RenderWindow window_;
        Device device_;
    public:
        App();
        
        ~App();
                
        void run();
    };
}
