#include "vulkan.hpp"

#ifdef DEBUG
#define DEBUG_PRINT(message) std::cerr << message << std::endl;
#else
#define DEBUG_PRINT(message)
#endif

namespace vk
{
#pragma mark - vk::debugCallback
    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData) noexcept
    {
        DEBUG_PRINT(pCallbackData->pMessage);
        return VK_FALSE;
    }

#pragma mark - vk::Instance
    Instance::Instance(const std::string& engineName, const std::string& applicationName)
    :
    engineName_(engineName),
    applicationName_(applicationName)
    {
        VkInstanceCreateInfo instanceCreateInfo = this->getVkInstanceCreateInfo();
        if (vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
        DEBUG_PRINT("Created instance!");
    }

    Instance::~Instance()
    {
        vkDestroyInstance(this->instance_, nullptr);
    }

    const VkInstance Instance::get() const noexcept
    {
        return this->instance_;
    }
    
    VkApplicationInfo Instance::getVkApplicationInfo() const noexcept
    {
        VkApplicationInfo applicationInfo{};
        applicationInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext               = nullptr;
        applicationInfo.pApplicationName    = this->applicationName_.c_str();
        applicationInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.pEngineName         = this->engineName_.c_str();
        applicationInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
        applicationInfo.apiVersion          = VK_API_VERSION_1_3;
        return applicationInfo;
    }
    
    VkDebugUtilsMessengerCreateInfoEXT Instance::getVkDebugUtilsMessengerCreateInfoEXT() const noexcept
    {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
        debugUtilsMessengerCreateInfoEXT.sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCreateInfoEXT.pNext              = nullptr;
        debugUtilsMessengerCreateInfoEXT.flags              = 0;
        debugUtilsMessengerCreateInfoEXT.messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.pfnUserCallback    = debugCallback;
        debugUtilsMessengerCreateInfoEXT.pUserData          = nullptr;
        return debugUtilsMessengerCreateInfoEXT;
    }
    
    VkInstanceCreateInfo Instance::getVkInstanceCreateInfo() const noexcept
    {
#ifdef __APPLE__
#define INSTANCE_FLAGS VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#else
#define INSTANCE_FLAGS 0
#endif
        
#ifdef DEBUG
#define INSTANCE_NEXT &debugUtilsMessengerCreateInfoEXT
#define INSTANCE_LAYERS_COUNT static_cast<std::uint32_t>(vk::layers.size())
#define INSTANCE_LAYERS_NAME vk::layers.data()
#else
#define INSTANCE_NEXT nullptr
#define INSTANCE_LAYERS_COUNT 0
#define INSTANCE_LAYERS_NAME nullptr
#endif
        
        VkApplicationInfo applicationInfo = this->getVkApplicationInfo();
        
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();
        
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.flags                    = INSTANCE_FLAGS;
        instanceCreateInfo.pNext                    = INSTANCE_NEXT;
        instanceCreateInfo.pApplicationInfo         = &applicationInfo;
        instanceCreateInfo.enabledLayerCount        = INSTANCE_LAYERS_COUNT;
        instanceCreateInfo.ppEnabledLayerNames      = INSTANCE_LAYERS_NAME;
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->extensions_.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->extensions_.data();
        return instanceCreateInfo;
    }
    
#pragma mark - vk::DebugUtilsMessenger
    DebugUtilsMessenger::DebugUtilsMessenger(const Instance& instance)
    :
    instance_(instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();

        if (vkCreateDebugUtilsMessengerEXT(this->instance_.get(), &debugUtilsMessengerCreateInfoEXT, nullptr, &this->debugUtilsMessengerEXT_) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
    }
    
    DebugUtilsMessenger::~DebugUtilsMessenger()
    {
        vkDestroyDebugUtilsMessengerEXT(this->instance_.get(), this->debugUtilsMessengerEXT_, nullptr);
    }
    
    const VkDebugUtilsMessengerEXT DebugUtilsMessenger::get() const noexcept
    {
        return this->debugUtilsMessengerEXT_;
    }
    
    VkDebugUtilsMessengerCreateInfoEXT DebugUtilsMessenger::getVkDebugUtilsMessengerCreateInfoEXT() const noexcept
    {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
        debugUtilsMessengerCreateInfoEXT.sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugUtilsMessengerCreateInfoEXT.pNext              = nullptr;
        debugUtilsMessengerCreateInfoEXT.flags              = 0;
        debugUtilsMessengerCreateInfoEXT.messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsMessengerCreateInfoEXT.pfnUserCallback    = debugCallback;
        debugUtilsMessengerCreateInfoEXT.pUserData          = nullptr;
        return debugUtilsMessengerCreateInfoEXT;
    }
}


