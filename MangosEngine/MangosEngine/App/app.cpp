#include "app.hpp"

namespace mgo
{
#pragma mark - App::RenderWindow
  
    App::RenderWindow::RenderWindow(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight)
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

    App::Device::QueueFamilyIndices::QueueFamilyIndices() noexcept
    :
    queuePriority(1.0f)
    {}

    bool App::Device::QueueFamilyIndices::findQueueFamilies(VkPhysicalDevice device) noexcept
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
                return true;
            }
            i++;
        }
        return false;
    }
    
#pragma mark - App::Device
    
    App::Device::Device(const std::string& appName)
    :
    appName(appName),
    engineName("Magnos Engine"),
    validationLayers(this->getValidationLayers()),
    instanceExtensions(this->getinstanceExtensions()),
    deviceExtensions(this->getdeviceExtensions()),
    instance(VK_NULL_HANDLE),
    debugMessenger(VK_NULL_HANDLE),
    physicalDevice(VK_NULL_HANDLE),
    device(VK_NULL_HANDLE)
    {
        this->setupVulkanInstance();
#if MGO_DEBUG
        this->setupDebugMessenger();
#endif
        this->setupDevice();
    }
    
    App::Device::~Device()
    {
        this->destroyDevice();
#if MGO_DEBUG
        this->destoryDebugMessenger();
#endif
        this->destoryVulkanInstance();
    }
    
    std::vector<const char*> App::Device::getValidationLayers() noexcept
    {
        return std::vector<const char*>({"VK_LAYER_KHRONOS_validation"});
    }
    
    std::vector<const char*> App::Device::getinstanceExtensions() noexcept
    {
        std::uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
                
        std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#if __APPLE__
        requiredExtensions.emplace_back("VK_KHR_portability_enumeration");
#endif
#if MGO_DEBUG
        requiredExtensions.emplace_back("VK_EXT_debug_utils");
#endif
        return requiredExtensions;
    }
    
    std::vector<const char*> App::Device::getdeviceExtensions() noexcept
    {
#if __APPLE__
        return std::vector<const char*>({"VK_KHR_portability_subset"});
#else
        return std::vector<const char*>({});
#endif
    }
    
    void App::Device::setupVulkanInstance()
    {
#if MGO_DEBUG
        this->checkValidationLayerSupport();
        this->checkInstanceExtensionSupport();
#endif
        VkApplicationInfo appInfo{};
        this->populateApplicationInfo(appInfo);
        
        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
        this->populateDebugUtilsMessengerCreateInfoEXT(DebugCreateInfo);
        
        VkInstanceCreateInfo createInfo{};
        this->populateInstanceCreateInfo(createInfo, &appInfo, &DebugCreateInfo);
        
        if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
    }
    
    void App::Device::setupDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        this->populateDebugUtilsMessengerCreateInfoEXT(createInfo);
        
        if (CreateDebugUtilsMessengerEXT(this->instance, &createInfo, nullptr, &this->debugMessenger) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
    }
    
    void App::Device::setupDevice()
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
                                    
            switch (phyicalDevicesProperties.deviceType)
            {
                case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)     : {phyicalDevicesCandidates.emplace(1, potentialPhyicalDevice); break;}
                case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)   : {phyicalDevicesCandidates.emplace(0, potentialPhyicalDevice); break;}
                default                                         : {throw std::runtime_error("Failed to find a suitable GPU!");  break;}
            };
        }
        this->physicalDevice = phyicalDevicesCandidates.begin()->second;

#if MGO_DEBUG
        this->checkDeviceExtensionSupport();
#endif
        if (!this->indices.findQueueFamilies(this->physicalDevice))
            throw std::runtime_error("failed to find queue family!");
        
        VkDeviceQueueCreateInfo queueCreateInfo;
        this->populateDeviceQueueCreateInfo(queueCreateInfo);
        
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        this->populatePhysicalDeviceFeatures(physicalDeviceFeatures);
        
        VkDeviceCreateInfo deviceCreateInfo;
        this->populateDeviceCreateInfo(deviceCreateInfo, &queueCreateInfo, &physicalDeviceFeatures);
        
        if (vkCreateDevice(this->physicalDevice, &deviceCreateInfo, nullptr, &this->device) != VK_SUCCESS)
            throw std::runtime_error("failed to create device!");

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
        debugCreateInfo.messageSeverity    = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugCreateInfo.messageSeverity   |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType        = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        debugCreateInfo.messageType       |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback    = this->debugCallback;
        debugCreateInfo.pUserData          = nullptr;
    }
    
    void App::Device::populateInstanceCreateInfo(VkInstanceCreateInfo& instanceCreateInfo,
                                                 const VkApplicationInfo* pAppInfo,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pDebugCreateInfo) const noexcept
    {
#if MGO_DEBUG
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = pDebugCreateInfo;
        instanceCreateInfo.pApplicationInfo         = pAppInfo;
        instanceCreateInfo.enabledLayerCount        = static_cast<std::uint32_t>(this->validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames      = this->validationLayers.data();
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->instanceExtensions.data();
#else
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = nullptr;
        instanceCreateInfo.pApplicationInfo         = pAppInfo;
        instanceCreateInfo.enabledLayerCount        = 0;
        instanceCreateInfo.ppEnabledLayerNames      = nullptr;
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->instanceExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->instanceExtensions.data();
#endif
#if __APPLE__
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
        instanceCreateInfo.flags = 0;
#endif
    }
    
    void App::Device::populateDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo) const noexcept
    {
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext               = nullptr;
        queueCreateInfo.flags               = 0;
        queueCreateInfo.queueFamilyIndex    = this->indices.graphicsFamily.value();
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &this->indices.queuePriority;
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
                                               const VkDeviceQueueCreateInfo* pQueueCreateInfo,
                                               const VkPhysicalDeviceFeatures* pPhysicalDeviceFeatures) const noexcept
    {
#if MGO_DEBUG
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = 1;
        deviceCreateInfo.pQueueCreateInfos          = pQueueCreateInfo;
        deviceCreateInfo.enabledLayerCount          = static_cast<std::uint32_t>(this->validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames        = this->validationLayers.data();
        deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(this->deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames    = this->deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures           = pPhysicalDeviceFeatures;
#else
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = 1;
        deviceCreateInfo.pQueueCreateInfos          = pQueueCreateInfo;
        deviceCreateInfo.enabledLayerCount          = 0;
        deviceCreateInfo.ppEnabledLayerNames        = nullptr;
        deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(this->deviceExtensions.size());
        deviceCreateInfo.ppEnabledExtensionNames    = this->deviceExtensions.data();
        deviceCreateInfo.pEnabledFeatures           = pPhysicalDeviceFeatures;
#endif
    }
    
    void App::Device::checkValidationLayerSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateInstanceLayerProperties(&propertyCound, nullptr);
        
        std::vector<VkLayerProperties> properties(propertyCound);
        vkEnumerateInstanceLayerProperties(&propertyCound, properties.data());
                
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->validationLayers)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.layerName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!allPropertiesFound)
                std::cerr << "Validation layer NOT supported: " << requiredProperty << std::endl;
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            std::cout << "All validation layers are supported!" << std::endl;
    }
    
    void App::Device::checkInstanceExtensionSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, nullptr);
        
        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->instanceExtensions)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!allPropertiesFound)
                std::cerr << "Instance extension NOT supported: " << requiredProperty << std::endl;
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            std::cout << "All instance extensions are supported!" << std::endl;
    }
    
    void App::Device::checkDeviceExtensionSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateDeviceExtensionProperties(this->physicalDevice, nullptr, &propertyCound, nullptr);

        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateDeviceExtensionProperties(this->physicalDevice, nullptr, &propertyCound, properties.data());

        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->deviceExtensions)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!allPropertiesFound)
                std::cerr << "Device extension NOT supported: " << requiredProperty << std::endl;
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            std::cout << "All device extensions are supported!" << std::endl;
    }
    
    VkResult App::Device::CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks* pAllocator,
                                                       VkDebugUtilsMessengerEXT* pDebugMessenger) noexcept
    {
        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
            
    }
    
    void App::Device::DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                                    VkDebugUtilsMessengerEXT debugMessenger,
                                                    const VkAllocationCallbacks* pAllocator) noexcept
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
    
    VKAPI_ATTR VkBool32 VKAPI_CALL App::Device::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                              VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                              void* pUserData) noexcept
    {
        std::cerr << pCallbackData->pMessage << std::endl;
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

