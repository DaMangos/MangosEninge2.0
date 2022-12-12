#pragma once
#include "mgo_glfw.hpp"
#include <vulkan/vulkan.h>
#include <map>
#include <set>
#include <fstream>
#include <array>
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
            
            const VkInstance& get() const noexcept;
            
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
            
            const VkDebugUtilsMessengerEXT& get() const noexcept;
            
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
            
            const VkSurfaceKHR& get() const noexcept;
            
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
            QueueFamilyIndices queueFamilyIndices_;
            const std::vector<const char*> extensions_;
            const Instance& instance_;
            const Surface& surface_;
            
        public:
            PhysicalDevice(const Instance& instance, const Surface& surface);
                        
            const VkPhysicalDevice& get() const noexcept;
            
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
            
            const VkDevice& get() const noexcept;
            
            const VkQueue& getGraphicsQueue() const noexcept;
            
            const VkQueue& getPresentQueue() const noexcept;
            
            void wait() const noexcept;

        private:
            VkDeviceQueueCreateInfo getDeviceQueueCreateInfo(std::uint32_t queueFamily, const float* pQueuePriority) const noexcept;
        };
        
#pragma mark - mgo::vk::Semaphore
        class Semaphore final
        {
        private:
            VkSemaphore semaphore_;
            const Device& device_;
            
        public:
            Semaphore(const Device& device);
            
            ~Semaphore() noexcept;
            
            const VkSemaphore& get() const noexcept;
        };
        
#pragma mark - mgo::vk::Fence
        class Fence final
        {
        private:
            VkFence fence_;
            const Device& device_;
            
        public:
            Fence(const Device& device);
            
            ~Fence() noexcept;
            
            const VkFence& get() const noexcept;
            
            void wait() const noexcept;
            
            void reset() const noexcept;
        };
        
#pragma mark - mgo::vk::Swapchain
        class Swapchain final
        {
        private:
             
            VkSwapchainKHR swapchain_;
            VkSurfaceCapabilitiesKHR surfaceCapabilities_;
            VkSurfaceFormatKHR surfaceFormat_;
            VkPresentModeKHR presentMode_;
            VkExtent2D extent_;
            const Surface& surface_;
            const PhysicalDevice& physicalDevice_;
            const Device& device_;
            
        public:
            Swapchain(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device);
            
            ~Swapchain() noexcept;
            
        private:
            void create();
            
            void destory();
            
        public:
            void recreate();

            const VkSwapchainKHR& get() const noexcept;

            VkSurfaceCapabilitiesKHR getVkSurfaceCapabilitiesKHR() const noexcept;
            
            VkSurfaceFormatKHR getVkSurfaceFormatKHR() const noexcept;
            
            VkPresentModeKHR getVkPresentModeKHR() const noexcept;
            
            VkExtent2D getVkExtent2D() const noexcept;
        };
        
#pragma mark - mgo::vk::ImageViews
        class ImageViews final
        {
        private:
            std::vector<VkImage> images_;
            std::vector<VkImageView> imageViews_;
            const Device& device_;
            
        public:
            ImageViews(const Device& device, const Swapchain& swapchain);
            
            ~ImageViews() noexcept;
            
        private:
            void create(const Swapchain& swapchain);
            
            void destory();
            
        public:
            void recreate(const Swapchain& swapchain);

            const std::vector<VkImageView>& get() const noexcept;
            
            std::size_t size() const noexcept;
        };
        
#pragma mark - mgo::vk::RenderPass
        class RenderPass final
        {
        private:
            VkRenderPass renderPass_;
            const Device& device_;
            const Swapchain& swapchain_;
            
        public:
            RenderPass(const Device& device, const Swapchain& swapchain);
            
            ~RenderPass() noexcept;
            
            const VkRenderPass& get() const noexcept;
            
        private:
            VkAttachmentDescription getVkAttachmentDescription() const noexcept;
            
            VkAttachmentReference getVkAttachmentReference() const noexcept;
            
            VkSubpassDescription getVkSubpassDescription(const VkAttachmentReference* pColorAttachmentReference) const noexcept;
        };
        
#pragma mark - mgo::vk::Framebuffers
        class Framebuffers final
        {
        private:
            std::vector<VkFramebuffer> framebuffers_;
            const Device& device_;
            ImageViews& imageViews_;
            
        public:
            Framebuffers(const Device& device, const Swapchain& swapchain, ImageViews& imageViews, const RenderPass& renderPass);
            
            ~Framebuffers() noexcept;
            
        private:
            void create(const Swapchain& swapchain, const RenderPass& renderPass);
            
            void destory();
            
        public:
            void recreate(const Swapchain& swapchain, const RenderPass& renderPass);
            
            const std::vector<VkFramebuffer>& get() const noexcept;
            
            std::size_t size() const noexcept;
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
            
            const VkPipelineLayout& get() const noexcept;
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
                
                const VkShaderModule& get() const noexcept;
            };
            
            VkPipeline pipeline_;
            const Device& device_;
            const RenderPass& renderPass_;
            const PipelineLayout& pipelineLayout_;
            
        public:
            Pipeline(const Device& device, const RenderPass& renderPass, const PipelineLayout& pipelineLayout);
            
            ~Pipeline() noexcept;
            
            const VkPipeline& get() const noexcept;
            
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
        
#pragma mark - mgo::vk::CommandPool
        class CommandPool final
        {
        private:
            VkCommandPool commandPool_;
            const PhysicalDevice& physicalDevice_;
            const Device& device_;
            
        public:
            CommandPool(const PhysicalDevice& physicalDevice, const Device& device);
            
            ~CommandPool() noexcept;
            
            const VkCommandPool& get() const noexcept;
        };
        
#pragma mark - mgo::vk::CommandBuffers
        class CommandBuffers final
        {
        public:
            static const std::size_t MAX_FRAMES_IN_FLIGHT = 2;

        private:
            std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> commandBuffers_;
            std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores_;
            std::array<Semaphore, MAX_FRAMES_IN_FLIGHT> renderFinishedSemaphores_;
            std::array<Fence, MAX_FRAMES_IN_FLIGHT> inFlightFences_;
            std::uint32_t imageIndex_;
            std::uint32_t currentFrame_;
            glfw::Window& window_;
            const Device& device_;
            Swapchain& swapchain_;
            RenderPass& renderPass_;
            Framebuffers& framebuffers_;
            const CommandPool& commandPool_;
            const Pipeline& pipeline_;
            
        public:
            
            CommandBuffers(glfw::Window& window,
                           const Device& device,
                           Swapchain& swapchain,
                           RenderPass& renderPass,
                           Framebuffers& framebuffers,
                           const Pipeline& pipeline,
                           const CommandPool& commandPool);
                        
            const std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT>& get() const noexcept;
            
            void draw();

        private:
            void getNextImageIndex();
            
            void beginCommandBuffer() const;
            
            void beginRenderPass() const noexcept;
            
            void bindPipline() const noexcept;
            
            void setViewport() const noexcept;
            
            void setScissor() const noexcept;

            void drawImage() const noexcept;
            
            void endRenderPass() const noexcept;
            
            void endCommandBuffer() const;
            
            void submitImage() const;
            
            void presentImage();
        };
    }
}
