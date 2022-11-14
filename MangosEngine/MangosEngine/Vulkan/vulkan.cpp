#include "vulkan.hpp"

#pragma mark - LOG(message)
#ifndef LOG
#ifdef DEBUG
#define LOG(message) std::cerr << message << std::endl
#else
#define LOG(message)
#endif
#endif
namespace vk
{
#pragma mark - vk::layers
#ifdef DEBUG
    const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
#define ENABLED_LAYERS_COUNT static_cast<std::uint32_t>(vk::layers.size())
#define ENABLED_LAYERS_NAME vk::layers.data()
#else
#define ENABLED_LAYERS_COUNT 0
#define ENABLED_LAYERS_NAME nullptr
#endif
    
#pragma mark - vk::Instance
    Instance::Instance(const std::string& engineName, const std::string& applicationName, const glfw::Window& window)
    :
    engineName_(engineName),
    applicationName_(applicationName),
    extensions_(this->getExtensions(window))
    {
#ifdef __APPLE__
#define INSTANCE_FLAGS VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#else
#define INSTANCE_FLAGS 0
#endif
#ifdef DEBUG
        this->checkInstanceExtensionSupport();
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();
#define INSTANCE_NEXT &debugUtilsMessengerCreateInfoEXT
#else
#define INSTANCE_NEXT nullptr
#endif
        VkApplicationInfo applicationInfo = this->getVkApplicationInfo();
        
        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.flags                    = INSTANCE_FLAGS;
        instanceCreateInfo.pNext                    = INSTANCE_NEXT;
        instanceCreateInfo.pApplicationInfo         = &applicationInfo;
        instanceCreateInfo.enabledLayerCount        = ENABLED_LAYERS_COUNT;
        instanceCreateInfo.ppEnabledLayerNames      = ENABLED_LAYERS_NAME;
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->extensions_.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->extensions_.data();
        
        if (vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
        LOG("Created instance!");
    }
    
    Instance::~Instance() noexcept
    {
        vkDestroyInstance(this->instance_, nullptr);
        LOG("Destroyed instance!");
    }
    
    const VkInstance Instance::get() const noexcept
    {
        return this->instance_;
    }
    
    std::vector<const char*> Instance::getExtensions(const glfw::Window& window) const noexcept
    {
        std::vector<const char*> extensions = window.getExtensions();
#ifdef __APPLE__
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
#ifdef DEBUG
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return extensions;
    }
    
    void Instance::checkInstanceExtensionSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, nullptr);
        
        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->extensions_)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!propertyFound)
                LOG("Instance extension NOT supported: " << requiredProperty);
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            LOG("All instance extensions are supported!");
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
        debugUtilsMessengerCreateInfoEXT.pfnUserCallback    = DebugUtilsMessenger::debugCallback;
        debugUtilsMessengerCreateInfoEXT.pUserData          = nullptr;
        return debugUtilsMessengerCreateInfoEXT;
    }
    
#pragma mark - vk::DebugUtilsMessenger
    DebugUtilsMessenger::DebugUtilsMessenger(const Instance& instance)
    :
    instance_(instance)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();
        if (DebugUtilsMessenger::createDebugUtilsMessengerEXT(this->instance_.get(),
                                                              &debugUtilsMessengerCreateInfoEXT,
                                                              nullptr,
                                                              &this->debugUtilsMessengerEXT_) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
        LOG("Created DebugUtilsMessenger!");
    }
    
    DebugUtilsMessenger::~DebugUtilsMessenger() noexcept
    {
        DebugUtilsMessenger::destroyDebugUtilsMessengerEXT(this->instance_.get(), this->debugUtilsMessengerEXT_, nullptr);
        LOG("Destroyed DebugUtilsMessenger!");
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
    
    VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                      VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                      void* pUserData) noexcept
    {
        LOG(pCallbackData->pMessage);
        return VK_FALSE;
    }
    
    VkResult DebugUtilsMessenger::createDebugUtilsMessengerEXT(VkInstance instance,
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
    
    void DebugUtilsMessenger::destroyDebugUtilsMessengerEXT(VkInstance instance,
                                                            VkDebugUtilsMessengerEXT debugMessenger,
                                                            const VkAllocationCallbacks* pAllocator) noexcept
    {
        auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
    
#pragma mark - vk::Surface
    Surface::Surface(const Instance& instance, const glfw::Window& window)
    :
    instance_(instance)
    {
        if (window.createSurface(this->instance_.get(), &this->surface_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface");
        LOG("Created surface");
    }
    
    Surface::~Surface() noexcept
    {
        vkDestroySurfaceKHR(this->instance_.get(), this->surface_, nullptr);
        LOG("Destoryed surface");
    }
    
    const VkSurfaceKHR Surface::get() const noexcept
    {
        return this->surface_;
    }
    
#pragma mark - vk::Device
    Device::Device(const Instance& instance, const Surface& surface)
    :
    instance_(instance),
    surface_(surface),
    extensions_(this->getExtensions())
    {
        this->pickPhysicalDevice();
        this->queueFamilyIndices_ = this->getQueueFamilyIndices(this->physicalDevice_, this->surface_.get(), 1.0f);
        
        std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
        std::set<uint32_t> queueFamilyIndices =
        {
            this->queueFamilyIndices_.graphicsFamily_.value(),
            this->queueFamilyIndices_.presentFamily_.value()
        };
        
        for (uint32_t queueFamilyIndex : queueFamilyIndices)
            deviceQueueCreateInfos.emplace_back(this->getDeviceQueueCreateInfo(queueFamilyIndex));
        
        VkPhysicalDeviceFeatures physicalDeviceFeatures = this->getPhysicalDeviceFeatures();
        
        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = static_cast<std::uint32_t>(deviceQueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos          = deviceQueueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount          = ENABLED_LAYERS_COUNT;
        deviceCreateInfo.ppEnabledLayerNames        = ENABLED_LAYERS_NAME;
        deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(this->extensions_.size());
        deviceCreateInfo.ppEnabledExtensionNames    = this->extensions_.data();
        deviceCreateInfo.pEnabledFeatures           = &physicalDeviceFeatures;
        
        if (vkCreateDevice(this->physicalDevice_, &deviceCreateInfo, nullptr, &this->device_) != VK_SUCCESS)
            throw std::runtime_error("failed to create device!");
        LOG("Created device!");
        
        vkGetDeviceQueue(this->device_, this->queueFamilyIndices_.graphicsFamily_.value(), 0, &this->graphicsQueue_);
        vkGetDeviceQueue(this->device_, this->queueFamilyIndices_.presentFamily_.value(), 0, &this->presentQueue_);
    }
    
    Device::~Device() noexcept
    {
        vkDestroyDevice(this->device_, nullptr);
        LOG("Destroyed device!");
    }
    
    std::vector<const char*> Device::getExtensions() const noexcept
    {
        std::vector<const char*> extensions;
        extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef __APPLE__
        extensions.emplace_back(VK_NV_GLSL_SHADER_EXTENSION_NAME);
        extensions.emplace_back("VK_KHR_portability_subset");
#endif
        return extensions;
    }
    
    const VkDevice Device::get() const noexcept
    {
        return this->device_;
    }
    
    Device::QueueFamilyIndices Device::getQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                                             VkSurfaceKHR surface,
                                                             float queuePriority) const noexcept
    {
        QueueFamilyIndices queueFamilyIndices{};
        queueFamilyIndices.queuePriority_ = queuePriority;
        
        std::uint32_t queueFamilyPropertyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
        
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());
        
        std::uint32_t queueFamilyIndex = 0;
        for (const auto& queueFamilyProperty : queueFamilyProperties)
        {
            VkBool32 presentSupport = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupport);
            
            if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                queueFamilyIndices.graphicsFamily_ = queueFamilyIndex;
            
            if (presentSupport)
                queueFamilyIndices.presentFamily_ = queueFamilyIndex;
            
            queueFamilyIndex++;
        }
        return queueFamilyIndices;
    }
    
    bool Device::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, bool logResults) const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCound, nullptr);
        
        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->extensions_)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!propertyFound && logResults)
                LOG("Instance extension NOT supported: " << requiredProperty);
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound  && logResults)
            LOG("All instance extensions are supported!");
        return allPropertiesFound;
    }
    
    std::uint8_t Device::rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept
    {
        std::uint8_t value = 0;
        
        VkPhysicalDeviceProperties phyicalDevicesProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &phyicalDevicesProperties);
        
        VkPhysicalDeviceFeatures phyicalDevicesFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &phyicalDevicesFeatures);
        
        std::uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, this->surface_.get(), &formatCount, nullptr);
        
        std::uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, this->surface_.get(), &presentModeCount, nullptr);
        
        QueueFamilyIndices indices = getQueueFamilyIndices(physicalDevice, this->surface_.get(), 1.0f);
        
        switch (phyicalDevicesProperties.deviceType)
        {
            case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)   : {value = 2; break;}
            case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)     : {value = 1; break;}
            default : return 0;
        };
        
        if (presentModeCount == 0 || formatCount == 0)
            return 0;
        
        if (!indices.graphicsFamily_.has_value() || !indices.presentFamily_.has_value())
            return 0;
        
        if (!this->checkPhysicalDeviceExtensionSupport(physicalDevice, false))
            return 0;
        
        return value;
    }
    
    void Device::pickPhysicalDevice()
    {
        std::uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance_.get(), &physicalDeviceCount, nullptr);
        
        if (physicalDeviceCount == 0)
            throw std::runtime_error("Failed to find a GPU with Vulkan support!");
        
        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
        vkEnumeratePhysicalDevices(this->instance_.get(), &physicalDeviceCount, physicalDevices.data());
        
        std::multimap<std::uint8_t, VkPhysicalDevice, std::greater<std::uint8_t>> physicalDevicesCandidates;
        
        for (auto physicalDevice : physicalDevices)
            physicalDevicesCandidates.emplace(this->rankPhysicalDevices(physicalDevice), physicalDevice);
#ifdef DEBUG
        this->checkPhysicalDeviceExtensionSupport(physicalDevicesCandidates.begin()->second, true);
#endif
        if (physicalDevicesCandidates.begin()->first == 0)
            throw std::runtime_error("Failed to find a suitable GPU!");
        
        this->physicalDevice_ = physicalDevicesCandidates.begin()->second;
        LOG("Found a suitable GPU!");
    }
    
    VkDeviceQueueCreateInfo Device::getDeviceQueueCreateInfo(std::uint32_t queueFamily) const noexcept
    {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.pNext               = nullptr;
        deviceQueueCreateInfo.flags               = 0;
        deviceQueueCreateInfo.queueFamilyIndex    = queueFamily;
        deviceQueueCreateInfo.queueCount          = 1;
        deviceQueueCreateInfo.pQueuePriorities    = &this->queueFamilyIndices_.queuePriority_;
        return deviceQueueCreateInfo;
    }
    
    VkPhysicalDeviceFeatures Device::getPhysicalDeviceFeatures() const noexcept
    {
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
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
        return physicalDeviceFeatures;
    }
}
