
#include "App.hpp"

namespace mgo
{
#pragma mark - App::RenderWindow
  
    App::RenderWindow::RenderWindow(const std::string& windowName, std::uint32_t windowHight, std::uint32_t windowWidth)
    :
    windowName(windowName), windowHight(windowHight), windowWidth(windowWidth), pWindow(nullptr)
    {
        glfwSetErrorCallback(this->errorCallback);
        
        if (!glfwInit())
            throw std::runtime_error("Failed to init GLFW!");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        this->pWindow = glfwCreateWindow(windowWidth, windowHight, windowName.c_str(), nullptr, nullptr);
        
        if (!this->pWindow)
            throw std::runtime_error("Failed to create GLFWwindow!");
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
    
    void App::RenderWindow::pollEvents() noexcept
    {
        glfwPollEvents();
    }

    
    void App::RenderWindow::errorCallback(int error, const char* description)
    {
        std::cerr << "GLFW error: " << description << std::endl;
    }
    
#pragma mark - App::Device::QueueFamilyIndices

    App::Device::QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device) noexcept
    {
        this->findQueueFamilies(device);
    }

    void App::Device::QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice device) noexcept
    {
        std::uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        
        std::uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                this->graphicsFamily = i;
                break;
            }
            i++;
        }
    }

    bool App::Device::QueueFamilyIndices::isComplete() const noexcept
    {
        return this->graphicsFamily.has_value();
    }
    
#pragma mark - App::Device
    
    App::Device::Device(const std::string& appName)
    :
    appName(appName),
    engineName("Magnos Engine"),
    requiredExtensions(this->getRequiredExtensions()),
    validationLayers(this->getValidationLayers()),
    instance(VK_NULL_HANDLE),
    debugMessenger(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE),
    device(VK_NULL_HANDLE)
    {
        this->setupVulkanInstance();
        if (MGO_DEBUG)
            this->setupDebugMessenger();
        this->setupPhysicalDevice();
        this->setupDevice();
    }
    
    App::Device::~Device()
    {
        this->destroyDevice();
        if (MGO_DEBUG)
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
        if (MGO_DEBUG)
        {
            this->checkValidationLayerSupport();
            this->checkExtensionSupport();
        }
            
        VkApplicationInfo appInfo;
        this->populateApplicationInfo(appInfo);
        
        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo;
        this->populateDebugUtilsMessengerCreateInfoEXT(DebugCreateInfo);
        
        VkInstanceCreateInfo createInfo;
        this->populateInstanceCreateInfo(createInfo, &appInfo, &DebugCreateInfo);
        
        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
    }
    
    void App::Device::setupDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        this->populateDebugUtilsMessengerCreateInfoEXT(createInfo);
        
        if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
    }
    
    void App::Device::setupPhysicalDevice()
    {
        std::uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, nullptr);
        
        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        
        std::vector<VkPhysicalDevice> phyicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(this->instance, &deviceCount, phyicalDevices.data());
                
        std::multimap<std::uint8_t, VkPhysicalDevice, std::greater<std::uint8_t>> phyicalDevicesCandidates;
        
        for (auto potentialPhyicalDevice : phyicalDevices)
        {
            VkPhysicalDeviceProperties phyicalDevicesProperties;
            vkGetPhysicalDeviceProperties(potentialPhyicalDevice, &phyicalDevicesProperties);
            
            VkPhysicalDeviceFeatures phyicalDevicesFeatures;
            vkGetPhysicalDeviceFeatures(potentialPhyicalDevice, &phyicalDevicesFeatures);
            
            QueueFamilyIndices queueFamily(potentialPhyicalDevice);
                        
            switch (phyicalDevicesProperties.deviceType)
            {
                case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)     : {phyicalDevicesCandidates.emplace(1, potentialPhyicalDevice); break;}
                case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)   : {phyicalDevicesCandidates.emplace(0, potentialPhyicalDevice); break;}
                default                                         : {throw std::runtime_error("Failed to find a suitable GPU!");  break;}
            };
        }
        this->physicalDevice = phyicalDevicesCandidates.begin()->second;
        
        this->indices.findQueueFamilies(this->physicalDevice);
    }
    
    void App::Device::setupDevice()
    {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo;
        this->populateDeviceQueueCreateInfo(deviceQueueCreateInfo);
        
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        //this->populatePhysicalDeviceFeatures(physicalDeviceFeatures);
        
        VkDeviceCreateInfo deviceCreateInfo;
        this->populateDeviceCreateInfo(deviceCreateInfo, &deviceQueueCreateInfo, &physicalDeviceFeatures);
        
        if (vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->device) != VK_SUCCESS)
            throw std::runtime_error("Failed to create logical device!");
        
        vkGetDeviceQueue(this->device, this->indices.graphicsFamily.value(), 0, &this->graphicsQueue);
    }

    void App::Device::destoryVulkanInstance() noexcept
    {
        vkDestroyInstance(this->instance, nullptr);
    }
    
    void App::Device::destoryDebugMessenger() noexcept
    {
        this->DestroyDebugUtilsMessengerEXT(this->instance, this->debugMessenger, nullptr);
    }
    
    void App::Device::destroyDevice() noexcept
    {
        vkDestroyDevice(this->device, nullptr);
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
    
    void App::Device::populateDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& debugCreateInfo) const noexcept
    {
        debugCreateInfo.sType              = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.pNext              = nullptr;
        debugCreateInfo.flags              = 0;
        debugCreateInfo.messageSeverity    = 0;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType        = 0;
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
    
    void App::Device::populateDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo) const noexcept
    {
        float queuePriority                 = 1.0f;
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        queueCreateInfo.pNext               = nullptr;
        queueCreateInfo.flags               = 0;
        queueCreateInfo.queueFamilyIndex    = this->indices.graphicsFamily.value();
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &queuePriority;
    }
    
    void App::Device::populatePhysicalDeviceFeatures(VkPhysicalDeviceFeatures& physicalDeviceFeatures) const noexcept
    {
        physicalDeviceFeatures.robustBufferAccess                       = 0;
        physicalDeviceFeatures.fullDrawIndexUint32                      = 0;
        physicalDeviceFeatures.imageCubeArray                           = 0;
        physicalDeviceFeatures.independentBlend                         = 0;
        physicalDeviceFeatures.geometryShader                           = 0;
        physicalDeviceFeatures.tessellationShader                       = 0;
        physicalDeviceFeatures.sampleRateShading                        = 0;
        physicalDeviceFeatures.dualSrcBlend                             = 0;
        physicalDeviceFeatures.logicOp                                  = 0;
        physicalDeviceFeatures.multiDrawIndirect                        = 0;
        physicalDeviceFeatures.drawIndirectFirstInstance                = 0;
        physicalDeviceFeatures.depthClamp                               = 0;
        physicalDeviceFeatures.depthBiasClamp                           = 0;
        physicalDeviceFeatures.fillModeNonSolid                         = 0;
        physicalDeviceFeatures.depthBounds                              = 0;
        physicalDeviceFeatures.wideLines                                = 0;
        physicalDeviceFeatures.largePoints                              = 0;
        physicalDeviceFeatures.alphaToOne                               = 0;
        physicalDeviceFeatures.multiViewport                            = 0;
        physicalDeviceFeatures.samplerAnisotropy                        = 0;
        physicalDeviceFeatures.textureCompressionETC2                   = 0;
        physicalDeviceFeatures.textureCompressionASTC_LDR               = 0;
        physicalDeviceFeatures.textureCompressionBC                     = 0;
        physicalDeviceFeatures.occlusionQueryPrecise                    = 0;
        physicalDeviceFeatures.pipelineStatisticsQuery                  = 0;
        physicalDeviceFeatures.vertexPipelineStoresAndAtomics           = 0;
        physicalDeviceFeatures.fragmentStoresAndAtomics                 = 0;
        physicalDeviceFeatures.shaderTessellationAndGeometryPointSize   = 0;
        physicalDeviceFeatures.shaderImageGatherExtended                = 0;
        physicalDeviceFeatures.shaderStorageImageExtendedFormats        = 0;
        physicalDeviceFeatures.shaderStorageImageMultisample            = 0;
        physicalDeviceFeatures.shaderStorageImageReadWithoutFormat      = 0;
        physicalDeviceFeatures.shaderStorageImageWriteWithoutFormat     = 0;
        physicalDeviceFeatures.shaderUniformBufferArrayDynamicIndexing  = 0;
        physicalDeviceFeatures.shaderSampledImageArrayDynamicIndexing   = 0;
        physicalDeviceFeatures.shaderStorageBufferArrayDynamicIndexing  = 0;
        physicalDeviceFeatures.shaderStorageImageArrayDynamicIndexing   = 0;
        physicalDeviceFeatures.shaderClipDistance                       = 0;
        physicalDeviceFeatures.shaderCullDistance                       = 0;
        physicalDeviceFeatures.shaderFloat64                            = 0;
        physicalDeviceFeatures.shaderInt64                              = 0;
        physicalDeviceFeatures.shaderInt16                              = 0;
        physicalDeviceFeatures.shaderResourceResidency                  = 0;
        physicalDeviceFeatures.shaderResourceMinLod                     = 0;
        physicalDeviceFeatures.sparseBinding                            = 0;
        physicalDeviceFeatures.sparseResidencyBuffer                    = 0;
        physicalDeviceFeatures.sparseResidencyImage2D                   = 0;
        physicalDeviceFeatures.sparseResidencyImage3D                   = 0;
        physicalDeviceFeatures.sparseResidency2Samples                  = 0;
        physicalDeviceFeatures.sparseResidency4Samples                  = 0;
        physicalDeviceFeatures.sparseResidency8Samples                  = 0;
        physicalDeviceFeatures.sparseResidency16Samples                 = 0;
        physicalDeviceFeatures.sparseResidencyAliased                   = 0;
        physicalDeviceFeatures.variableMultisampleRate                  = 0;
        physicalDeviceFeatures.inheritedQueries                         = 0;
    }

    void App::Device::populateDeviceCreateInfo(VkDeviceCreateInfo& deviceCreateInfo,
                                               VkDeviceQueueCreateInfo* queueCreateInfo,
                                               VkPhysicalDeviceFeatures* physicalDeviceFeatures) const noexcept
    {
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = 1;
        deviceCreateInfo.pQueueCreateInfos          = queueCreateInfo;
        deviceCreateInfo.enabledLayerCount          = MGO_DEBUG ? static_cast<std::uint32_t>(this->validationLayers.size()) : 0;
        deviceCreateInfo.ppEnabledLayerNames        = MGO_DEBUG ? this->validationLayers.data() : nullptr;
        deviceCreateInfo.enabledExtensionCount      = 0;
        deviceCreateInfo.ppEnabledExtensionNames    = nullptr;
        deviceCreateInfo.pEnabledFeatures           = physicalDeviceFeatures;
    }

    
    void App::Device::checkExtensionSupport() const noexcept
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
                std::cerr << "Extension NOT supported: " << requiredExtension << std::endl;
            allExtensionFound = extensionFound ? allExtensionFound : false;
        }
        if (allExtensionFound)
            std::cout << "All requied extension are supported!" << std::endl;
    }
    
    void App::Device::checkValidationLayerSupport() const noexcept
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
    :
    appName("Mangos"),
    window(this->appName, 500, 500),
    device(this->appName)
    {}
    
    App::~App()
    {}
    
    void App::run()
    {
        while (!this->window.shouldClose())
            this->window.pollEvents();
    }
}

