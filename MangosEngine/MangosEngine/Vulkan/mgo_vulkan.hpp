#pragma once
#include "mgo_glfw.hpp"
#include <vulkan/vulkan.h>
#include <map>
#include <set>
#include <fstream>
namespace mgo
{
    namespace vk
    {
#pragma mark - mgo::vk::Instance
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
            
            VkInstance get() const noexcept;
            
        private:
            void checkInstanceExtensionSupport() const noexcept;
            
            std::vector<const char*> getExtensions(const glfw::Window& window) const noexcept;
            
            VkApplicationInfo getVkApplicationInfo() const noexcept;
            
            VkDebugUtilsMessengerCreateInfoEXT getVkDebugUtilsMessengerCreateInfoEXT() const noexcept;
        };
        
#pragma mark - mgo::vk::DebugUtilsMessenger
        class DebugUtilsMessenger final
        {
        private:
            VkDebugUtilsMessengerEXT debugUtilsMessengerEXT_;
            const Instance& instance_;
            
        public:
            DebugUtilsMessenger(const Instance& instance);
            
            ~DebugUtilsMessenger() noexcept;
            
            VkDebugUtilsMessengerEXT get() const noexcept;
            
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
        
#pragma mark - mgo::vk::Surface
        class PhysicalDevice;
        class Surface final
        {
        private:
            VkSurfaceKHR surface_;
            VkExtent2D extent_;
            const Instance& instance_;
            const glfw::Window& window_;
            
        public:
            Surface(const Instance& instance, const glfw::Window& window);
            
            ~Surface() noexcept;
            
            VkSurfaceKHR get() const noexcept;
            
            VkSurfaceCapabilitiesKHR getVkSurfaceCapabilitiesKHR(const PhysicalDevice& physicalDevice) const noexcept;
            
            VkSurfaceFormatKHR getVkSurfaceFormatKHR(const PhysicalDevice& physicalDevice) const noexcept;
            
            VkPresentModeKHR getVkPresentModeKHR(const PhysicalDevice& physicalDevice) const noexcept;
            
            VkExtent2D getVkExtent2D(const PhysicalDevice& physicalDevice) const noexcept;
        };
        
#pragma mark - mgo::vk::PhysicalDevice
        class PhysicalDevice final
        {
        public:
            struct QueueFamilyIndices
            {
                std::optional<std::uint32_t> graphicsFamily_;
                std::optional<std::uint32_t> presentFamily_;
                float priority_;
            };
            
            struct UniqueQueueFamilyIndices
            {
                std::set<std::uint32_t> families_;
                float priority_;
            };
            
        private:
            VkPhysicalDevice physicalDevice_;
            VkQueue graphicsQueue_;
            VkQueue presentQueue_;
            QueueFamilyIndices queueFamilyIndices_;
            const std::vector<const char*> extensions_;
            const Instance& instance_;
            const Surface& surface_;

        public:
            PhysicalDevice(const Instance& instance, const Surface& surface);
            
            ~PhysicalDevice() noexcept;
            
            VkPhysicalDevice get() const noexcept;
            
            std::vector<const char*> getExtensions() const noexcept;
            
            QueueFamilyIndices getQueueFamilyIndices() const noexcept;
            
            UniqueQueueFamilyIndices getUniqueQueueFamilyIndices() const noexcept;

            VkPhysicalDeviceFeatures getPhysicalDeviceFeatures() const noexcept;

        private:
            std::uint8_t rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept;
            
            bool checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, bool logResults) const noexcept;
            
            QueueFamilyIndices findQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                                      VkSurfaceKHR surface,
                                                      float queuePriority) const noexcept;
        };
        
#pragma mark - mgo::vk::Device
        class Device final
        {
        private:
            VkDevice device_;
            VkQueue graphicsQueue_;
            VkQueue presentQueue_;
            const Instance& instance_;
            const Surface& surface_;
            const PhysicalDevice& physicalDevice_;
            
        public:
            Device(const Instance& instance, const Surface& surface, const PhysicalDevice& physicalDevice);
            
            ~Device() noexcept;
            
            VkDevice get() const noexcept;
                        
        private:
            VkDeviceQueueCreateInfo getDeviceQueueCreateInfo(std::uint32_t queueFamily, const float* pQueuePriority) const noexcept;
        };
        
#pragma mark - mgo::vk::Swapchain
        class Swapchain final
        {
        private:
            VkSwapchainKHR swapchain_;
            const Surface& surface_;
            const PhysicalDevice& physicalDevice_;
            const Device& device_;
            
        public:
            Swapchain(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device);
            
            ~Swapchain() noexcept;
            
            VkSwapchainKHR get() const noexcept;
        };
        
#pragma mark - mgo::vk::ImageViews
        class ImageViews final
        {
        private:
            std::vector<VkImage> images_;
            std::vector<VkImageView> imageViews_;
            const Surface& surface_;
            const PhysicalDevice& physicalDevice_;
            const Device& device_;
            const Swapchain& swapchain_;
            
        public:
            ImageViews(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device, const Swapchain& swapchain);
            
            ~ImageViews() noexcept;
            
            std::vector<VkImageView> get() const noexcept;
            
        private:
            VkImageViewCreateInfo getVkImageViewCreateInfo(VkImage image) const noexcept;
        };
        
#pragma mark - mgo::vk::RenderPass
        class RenderPass final
        {
        private:
            VkRenderPass renderPass_;
            const Surface& surface_;
            const PhysicalDevice& physicalDevice_;
            const Device& device_;

        public:
            RenderPass(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device);
            
            ~RenderPass() noexcept;
            
            VkRenderPass get() const noexcept;
            
        private:
            VkAttachmentDescription getVkAttachmentDescription() const noexcept;
            
            VkAttachmentReference getVkAttachmentReference() const noexcept;
            
            VkSubpassDescription getVkSubpassDescription(const VkAttachmentReference* pColorAttachmentReference) const noexcept;
        };
#pragma mark - mgo::vk::PipelineLayout
        class PipelineLayout
        {
        private:
            VkPipelineLayout pipelineLayout_;
            const Device& device_;

        public:
            PipelineLayout(const Device& device);
            
            ~PipelineLayout() noexcept;
            
            VkPipelineLayout get() const noexcept;
        };
        
#pragma mark - mgo::vk::Pipeline
        class Pipeline final
        {
        private:
#pragma mark - mgo::vk::Pipeline::ShaderModule
            class ShaderModule final
            {
            private:
                VkShaderModule shaderModule_;
                const Device& device_;
                
            public:
                ShaderModule(const std::string& path, const Device& device);
                
                ~ShaderModule() noexcept;
                
                VkShaderModule get() const noexcept;
            };
            
            VkPipeline pipeline_;
            const Device& device_;
            const RenderPass& renderPass_;
            const PipelineLayout& pipelineLayout_;
            
        public:
            Pipeline(const Device& device, const RenderPass& renderPass, const PipelineLayout& pipelineLayout);
            
            ~Pipeline() noexcept;
            
            VkPipeline get() const noexcept;
            
        private:
            VkPipelineShaderStageCreateInfo getVkPipelineShaderStageCreateInfo(const ShaderModule& shaderModule,
                                                                               VkShaderStageFlagBits stage) const noexcept;
        
            VkPipelineVertexInputStateCreateInfo getVkPipelineVertexInputStateCreateInfo() const noexcept;
            
            VkPipelineInputAssemblyStateCreateInfo getVkPipelineInputAssemblyStateCreateInfo() const noexcept;
            
            VkPipelineViewportStateCreateInfo getVkPipelineViewportStateCreateInfo() const noexcept;
            
            VkPipelineRasterizationStateCreateInfo getVkPipelineRasterizationStateCreateInfo() const noexcept;
            
            VkPipelineMultisampleStateCreateInfo getVkPipelineMultisampleStateCreateInfo() const noexcept;
            
            VkPipelineColorBlendAttachmentState getVkPipelineColorBlendAttachmentState() const noexcept;

            VkPipelineColorBlendStateCreateInfo
            getVkPipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) const noexcept;

            VkPipelineDynamicStateCreateInfo getVkPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamicStates) const noexcept;
        };
        
    }
}
