
#include "App.hpp"

namespace mgo
{
#pragma mark - App::RenderWindow
  
    App::RenderWindow::RenderWindow(const std::string& windowName, std::uint32_t windowHight, std::uint32_t windowWidth)
    :
    windowName(windowName), windowHight(windowHight), windowWidth(windowWidth), pWindow(nullptr)
    {
        if (!glfwInit())
            throw std::runtime_error("failed to init GLFW!");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        this->pWindow = glfwCreateWindow(windowWidth, windowHight, windowName.c_str(), nullptr, nullptr);
        
        if (!this->pWindow)
            throw std::runtime_error("failed to create GLFWwindow!");
    }
    
    App::RenderWindow::~RenderWindow() noexcept
    {
        glfwDestroyWindow(this->pWindow);
        glfwTerminate();
    }
    
    bool App::RenderWindow::shouldClose() const noexcept
    {
        return glfwWindowShouldClose(this->pWindow);
    }
    
        
#pragma mark - App::Device
    
    App::Device::Device(const std::string& appName)
    :
    appName(appName),
    engineName("Magnos Engine"),
    requiredExtensions(this->getRequiredExtensions()),
    validationLayers(this->getValidationLayers())
    {
        this->setupVulkanInstance();
        this->setupDebugMessenger();
    }
    
    App::Device::~Device()
    {
        this->destoryDebugMessenger();
        this->destoryVulkanInstance();
    }
    
    std::vector<const char*> App::Device::getRequiredExtensions() noexcept
    {
        std::uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (MGO_DEBUG)
            requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        if (__APPLE__)
            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        return requiredExtensions;
    }
    
    std::vector<const char*> App::Device::getValidationLayers() noexcept
    {
        return std::vector<const char*>({"VK_LAYER_KHRONOS_validation"});
    }
    
    void App::Device::setupVulkanInstance()
    {
        VkApplicationInfo appInfo;
        this->populateApplicationInfo(appInfo);
        
        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo;
        this->populateDebugMessengerCreateInfo(DebugCreateInfo);
        
        VkInstanceCreateInfo createInfo;
        this->populateInstanceCreateInfo(createInfo, &appInfo, &DebugCreateInfo);
        
        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
    }
    
    void App::Device::setupDebugMessenger()
    {
        if (MGO_DEBUG)
        {
            VkDebugUtilsMessengerCreateInfoEXT createInfo;
            populateDebugMessengerCreateInfo(createInfo);
            
            if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS)
                throw std::runtime_error("Failed to set up debug messenger!");
        }
    }
    
    void App::Device::destoryVulkanInstance() noexcept
    {
        vkDestroyInstance(this->instance, nullptr);
    }
    
    void App::Device::destoryDebugMessenger() noexcept
    {
        if (MGO_DEBUG)
            this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
    }
    void App::Device::populateApplicationInfo(VkApplicationInfo& appInfo) const noexcept
    {
        appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext               = nullptr;
        appInfo.pApplicationName    = engineName.c_str();
        appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName         = appName.c_str();
        appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion          = VK_API_VERSION_1_3;
    }
    
    void App::Device::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo) const noexcept
    {
        debugCreateInfo.sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext              = nullptr;
        debugCreateInfo.flags              = 0;
        debugCreateInfo.messageSeverity    = 0;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType        = 0;
        debugCreateInfo.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        debugCreateInfo.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugCreateInfo.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback    = this->debugCallback;
        debugCreateInfo.pUserData          = nullptr;
    }
    
    void App::Device::populateInstanceCreateInfo(VkInstanceCreateInfo& instanceCreateInfo,
                                                 VkApplicationInfo* pApplicationInfo,
                                                 VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const noexcept
    {
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = MGO_DEBUG ? pDebugCreateInfo : nullptr;
        instanceCreateInfo.flags                    = __APPLE__ ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0;
        instanceCreateInfo.pApplicationInfo         = pApplicationInfo;
        instanceCreateInfo.enabledLayerCount        = MGO_DEBUG ? static_cast<std::uint32_t>(this->validationLayers.size()) : 0;
        instanceCreateInfo.ppEnabledLayerNames      = MGO_DEBUG ? this->validationLayers.data() : nullptr;
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->requiredExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->requiredExtensions.data();
    }
    
    bool App::Device::checkExtensionSupport() const noexcept
    {
        std::uint32_t supportedExtensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
        
        std::vector<VkExtensionProperties> supportedExtensions(supportedExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());
            
        bool allExtensionFound = true;
        for (const auto& requiredExtension : this->requiredExtensions)
        {
            bool extensionFound = false;
            for (const auto& supportedExtension : supportedExtensions)
                if (std::strcmp(requiredExtension, supportedExtension.extensionName) == 0)
                {
                    extensionFound = true;
                    break;
                }
            if (!extensionFound)
                std::cout << "Extension NOT supported: " << requiredExtension << std::endl;
            allExtensionFound = extensionFound ? allExtensionFound : false;
        }
        if (allExtensionFound)
            std::cout << "All requied extension are supported!" << std::endl;
        
        return allExtensionFound;
    }
    
    bool App::Device::checkValidationLayerSupport() const noexcept
    {
        std::uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayerProperties(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayerProperties.data());

        bool allLayerFound = true;
        for (const auto& validationLayer : this->validationLayers)
        {
            bool layerFound = false;
            for (const auto& availablelayerProperty : availableLayerProperties)
                if (std::strcmp(validationLayer, availablelayerProperty.layerName) == 0)
                {
                    layerFound = true;
                    break;
                }
            if (!layerFound)
                std::cout << "Extension NOT supported: " << validationLayer << std::endl;
            allLayerFound = layerFound ? allLayerFound : false;
        }
        if (allLayerFound)
            std::cout << "All validation layers are supported!" << std::endl;
        return allLayerFound;
    }
    
    VkResult App::Device::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDebugUtilsMessengerEXT* pDebugMessenger) noexcept
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
            
    }
    
    void App::Device::DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                    VkDebugUtilsMessengerEXT debugMessenger,
                                                    const VkAllocationCallbacks* pAllocator) noexcept
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
    
    VKAPI_ATTR VkBool32 VKAPI_CALL App::Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData) noexcept
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    
   
    
#pragma mark - App
    App::App()
    {}
    
    App::~App()
    {}
    
    void App::run()
    {}
}

