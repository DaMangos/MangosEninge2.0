#include "vulkan.hpp"
namespace mgo
{
    namespace vk
    {
#pragma mark - mgo::vk::layers
#ifdef DEBUG
        const std::vector<const char*> layers = {"VK_LAYER_KHRONOS_validation"};
#define MGO_VK_ENABLED_LAYERS_COUNT static_cast<std::uint32_t>(vk::layers.size())
#define MGO_VK_ENABLED_LAYERS_NAME vk::layers.data()
#else
#define MGO_VK_ENABLED_LAYERS_COUNT 0
#define MGO_VK_ENABLED_LAYERS_NAME nullptr
#endif
        
#pragma mark - mgo::vk::Instance
        Instance::Instance(const std::string& engineName, const std::string& applicationName, const glfw::Window& window)
        :
        engineName_(engineName),
        applicationName_(applicationName),
        extensions_(this->getExtensions(window))
        {
#ifdef __APPLE__
#define MGO_VK_INSTANCE_FLAGS VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
#else
#define MGO_VK_INSTANCE_FLAGS 0
#endif
#ifdef DEBUG
            this->checkInstanceExtensionSupport();
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();
#define MGO_VK_INSTANCE_NEXT &debugUtilsMessengerCreateInfoEXT
#else
#define MGO_VK_INSTANCE_NEXT nullptr
#endif
            VkApplicationInfo applicationInfo = this->getVkApplicationInfo();
            
            VkInstanceCreateInfo instanceCreateInfo{};
            instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.flags                    = MGO_VK_INSTANCE_FLAGS;
            instanceCreateInfo.pNext                    = MGO_VK_INSTANCE_NEXT;
            instanceCreateInfo.pApplicationInfo         = &applicationInfo;
            instanceCreateInfo.enabledLayerCount        = MGO_VK_ENABLED_LAYERS_COUNT;
            instanceCreateInfo.ppEnabledLayerNames      = MGO_VK_ENABLED_LAYERS_NAME;
            instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->extensions_.size());
            instanceCreateInfo.ppEnabledExtensionNames  = this->extensions_.data();
            
            if (vkCreateInstance(&instanceCreateInfo, nullptr, &this->instance_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create mgo::vk::Instance!");
            MGO_LOG("Created mgo::vk::Instance!");
        }
        
        Instance::~Instance() noexcept
        {
            vkDestroyInstance(this->instance_, nullptr);
            MGO_LOG("Destroyed mgo::vk::Instance!");
        }
        
        VkInstance Instance::get() const noexcept
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
                    MGO_LOG("mgo::vk::Instance extension NOT supported: " << requiredProperty);
                allPropertiesFound = propertyFound ? allPropertiesFound : false;
            }
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
        
#pragma mark - mgo::vk::DebugUtilsMessenger
        DebugUtilsMessenger::DebugUtilsMessenger(const Instance& instance)
        :
        instance_(instance)
        {
            VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT = this->getVkDebugUtilsMessengerCreateInfoEXT();
            if (DebugUtilsMessenger::createDebugUtilsMessengerEXT(this->instance_.get(),
                                                                  &debugUtilsMessengerCreateInfoEXT,
                                                                  nullptr,
                                                                  &this->debugUtilsMessengerEXT_) != VK_SUCCESS)
                throw std::runtime_error("Failed to set up mgo::vk::DebugUtilsMessenger!");
            MGO_LOG("Created mgo::vk::DebugUtilsMessenger!");
        }
        
        DebugUtilsMessenger::~DebugUtilsMessenger() noexcept
        {
            DebugUtilsMessenger::destroyDebugUtilsMessengerEXT(this->instance_.get(), this->debugUtilsMessengerEXT_, nullptr);
            MGO_LOG("Destroyed mgo::vk::DebugUtilsMessenger!");
        }
        
        VkDebugUtilsMessengerEXT DebugUtilsMessenger::get() const noexcept
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
            MGO_LOG(pCallbackData->pMessage);
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
#pragma mark - mgo::vk::Surface
        Surface::Surface(const Instance& instance, const glfw::Window& window)
        :
        instance_(instance),
        window_(window)
        {
            if (this->window_.createSurface(this->instance_.get(), &this->surface_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create mgo::vk::Surface");
            MGO_LOG("Created mgo::vk::Surface!");
        }
        
        Surface::~Surface() noexcept
        {
            vkDestroySurfaceKHR(this->instance_.get(), this->surface_, nullptr);
            MGO_LOG("Destoryed mgo::vk::Surface!");
        }
        
        VkSurfaceKHR Surface::get() const noexcept
        {
            return this->surface_;
        }
        
        VkSurfaceCapabilitiesKHR Surface::getVkSurfaceCapabilitiesKHR(const PhysicalDevice& physicalDevice) const noexcept
        {
            VkSurfaceCapabilitiesKHR surfaceCapabilities;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.get(), this->surface_, &surfaceCapabilities);
            return surfaceCapabilities;
        }
        
        VkSurfaceFormatKHR Surface::getVkSurfaceFormatKHR(const PhysicalDevice& physicalDevice) const noexcept
        {
            std::uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.get(), this->surface_, &formatCount, nullptr);
            
            std::vector<VkSurfaceFormatKHR> surfaceFormats(static_cast<std::size_t>(formatCount));
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice.get(), this->surface_, &formatCount, surfaceFormats.data());
            
            for (const auto& availableSurfaceFormat : surfaceFormats)
                if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                    return availableSurfaceFormat;
            return surfaceFormats[0];
        }
        
        VkPresentModeKHR Surface::getVkPresentModeKHR(const PhysicalDevice& physicalDevice) const noexcept
        {
            std::uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.get(), this->surface_, &presentModeCount, nullptr);
            
            std::vector<VkPresentModeKHR> presentModes(static_cast<std::size_t>(presentModeCount));
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice.get(), this->surface_, &presentModeCount, presentModes.data());
            
            for (const auto& availablePresentMode : presentModes)
                if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                    return availablePresentMode;
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        
        VkExtent2D Surface::getVkExtent2D(const PhysicalDevice& physicalDevice) const noexcept
        {
            VkSurfaceCapabilitiesKHR surfaceCapabilities = this->getVkSurfaceCapabilitiesKHR(physicalDevice);
            
            return {std::clamp(this->window_.GetFramebufferSize().width,
                               surfaceCapabilities.minImageExtent.width,
                               surfaceCapabilities.maxImageExtent.width),
                std::clamp(this->window_.GetFramebufferSize().height,
                           surfaceCapabilities.minImageExtent.height,
                           surfaceCapabilities.maxImageExtent.height)};
        }
        
#pragma mark - mgo::vk::PhysicalDevice
        PhysicalDevice::PhysicalDevice(const Instance& instance, const Surface& surface)
        :
        instance_(instance),
        surface_(surface),
        extensions_(this->getExtensions())
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
            MGO_LOG("Created mgo::vk::PhysicalDevice!");
            
            this->queueFamilyIndices_ = findQueueFamilyIndices(this->physicalDevice_, this->surface_.get(), 1.0f);
        }
        
        PhysicalDevice::~PhysicalDevice() noexcept
        {
            MGO_LOG("Destroyed mgo::vk::PhysicalDevice!");
        }
        
        VkPhysicalDevice PhysicalDevice::get() const noexcept
        {
            return this->physicalDevice_;
        }
        
        std::vector<const char*> PhysicalDevice::getExtensions() const noexcept
        {
            std::vector<const char*> extensions;
            extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#ifdef __APPLE__
            extensions.emplace_back(VK_NV_GLSL_SHADER_EXTENSION_NAME);
            extensions.emplace_back("VK_KHR_portability_subset");
#endif
            return extensions;
        }
        
        PhysicalDevice::QueueFamilyIndices PhysicalDevice::getQueueFamilyIndices() const noexcept
        {
            return this->queueFamilyIndices_;
        }
        
        PhysicalDevice::UniqueQueueFamilyIndices PhysicalDevice::getUniqueQueueFamilyIndices() const noexcept
        {
            UniqueQueueFamilyIndices uniqueQueueFamilyIndices{};
            uniqueQueueFamilyIndices.families_.emplace(this->queueFamilyIndices_.graphicsFamily_.value());
            uniqueQueueFamilyIndices.families_.emplace(this->queueFamilyIndices_.presentFamily_.value());
            uniqueQueueFamilyIndices.priority_ = this->queueFamilyIndices_.priority_;
            return uniqueQueueFamilyIndices;
        }
        
        VkPhysicalDeviceFeatures PhysicalDevice::getPhysicalDeviceFeatures() const noexcept
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
        
        std::uint8_t PhysicalDevice::rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept
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
            
            QueueFamilyIndices queueFamilyindices = findQueueFamilyIndices(physicalDevice, this->surface_.get(), 1.0f);
            
            switch (phyicalDevicesProperties.deviceType)
            {
                case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)   : {value = 2; break;}
                case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)     : {value = 1; break;}
                default : return 0;
            };
            
            if (presentModeCount == 0 || formatCount == 0)
                return 0;
            
            if (!queueFamilyindices.graphicsFamily_.has_value() || !queueFamilyindices.presentFamily_.has_value())
                return 0;
            
            if (!this->checkPhysicalDeviceExtensionSupport(physicalDevice, false))
                return 0;
            
            return value;
        }
        
        bool PhysicalDevice::checkPhysicalDeviceExtensionSupport(VkPhysicalDevice physicalDevice, bool logResults) const noexcept
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
                    MGO_LOG("mgo::vk::PhysicalDevice extension NOT supported: " << requiredProperty);
                allPropertiesFound = propertyFound ? allPropertiesFound : false;
            }
            return allPropertiesFound;
        }
        
        PhysicalDevice::QueueFamilyIndices PhysicalDevice::findQueueFamilyIndices(VkPhysicalDevice physicalDevice,
                                                                                  VkSurfaceKHR surface,
                                                                                  float queuePriority) const noexcept
        {
            QueueFamilyIndices queueFamilyIndices{};
            queueFamilyIndices.priority_ = queuePriority;
            
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
        
#pragma mark - mgo::vk::Device
        Device::Device(const Instance& instance, const Surface& surface, const PhysicalDevice& physicalDevice)
        :
        instance_(instance),
        surface_(surface),
        physicalDevice_(physicalDevice)
        {
            PhysicalDevice::UniqueQueueFamilyIndices uniqueQueueFamilyIndices = this->physicalDevice_.getUniqueQueueFamilyIndices();
            
            std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
            for (std::uint32_t uniqueQueueFamily : uniqueQueueFamilyIndices.families_)
                deviceQueueCreateInfos.emplace_back(this->getDeviceQueueCreateInfo(uniqueQueueFamily, &uniqueQueueFamilyIndices.priority_));
            
            std::vector<const char*> extensions = this->physicalDevice_.getExtensions();
            VkPhysicalDeviceFeatures physicalDeviceFeatures = this->physicalDevice_.getPhysicalDeviceFeatures();
            
            VkDeviceCreateInfo deviceCreateInfo{};
            deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceCreateInfo.pNext                      = nullptr;
            deviceCreateInfo.flags                      = 0;
            deviceCreateInfo.queueCreateInfoCount       = static_cast<std::uint32_t>(deviceQueueCreateInfos.size());
            deviceCreateInfo.pQueueCreateInfos          = deviceQueueCreateInfos.data();
            deviceCreateInfo.enabledLayerCount          = MGO_VK_ENABLED_LAYERS_COUNT;
            deviceCreateInfo.ppEnabledLayerNames        = MGO_VK_ENABLED_LAYERS_NAME;
            deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(extensions.size());
            deviceCreateInfo.ppEnabledExtensionNames    = extensions.data();
            deviceCreateInfo.pEnabledFeatures           = &physicalDeviceFeatures;
            
            if (vkCreateDevice(this->physicalDevice_.get(), &deviceCreateInfo, nullptr, &this->device_) != VK_SUCCESS)
                throw std::runtime_error("failed to create mgo::vk::Device!");
            MGO_LOG("Created mgo::vk::Device!");
            
            vkGetDeviceQueue(this->device_, this->physicalDevice_.getQueueFamilyIndices().graphicsFamily_.value(), 0, &this->graphicsQueue_);
            vkGetDeviceQueue(this->device_, this->physicalDevice_.getQueueFamilyIndices().presentFamily_.value(), 0, &this->presentQueue_);
        }
        
        Device::~Device() noexcept
        {
            vkDestroyDevice(this->device_, nullptr);
            MGO_LOG("Destroyed mgo::vk::Device!");
        }
        
        VkDevice Device::get() const noexcept
        {
            return this->device_;
        }
        
        VkDeviceQueueCreateInfo Device::getDeviceQueueCreateInfo(std::uint32_t queueFamily, const float* pQueuePriority) const noexcept
        {
            VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
            deviceQueueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.pNext               = nullptr;
            deviceQueueCreateInfo.flags               = 0;
            deviceQueueCreateInfo.queueFamilyIndex    = queueFamily;
            deviceQueueCreateInfo.queueCount          = 1;
            deviceQueueCreateInfo.pQueuePriorities    = pQueuePriority;
            return deviceQueueCreateInfo;
        }
        
#pragma mark - mgo::vk::Swapchain
        Swapchain::Swapchain(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device)
        :
        surface_(surface),
        physicalDevice_(physicalDevice),
        device_(device)
        {
            VkSurfaceCapabilitiesKHR surfaceCapabilities    = this->surface_.getVkSurfaceCapabilitiesKHR(this->physicalDevice_);
            VkSurfaceFormatKHR surfaceFormat                = this->surface_.getVkSurfaceFormatKHR(this->physicalDevice_);
            VkPresentModeKHR presentMode                    = this->surface_.getVkPresentModeKHR(this->physicalDevice_);
            VkExtent2D extent                               = this->surface_.getVkExtent2D(this->physicalDevice_);
            
            std::set<std::uint32_t> UniqueQueueFamilyIndices = this->physicalDevice_.getUniqueQueueFamilyIndices().families_;
            std::vector<std::uint32_t> queueFamilyIndices(UniqueQueueFamilyIndices.begin(), UniqueQueueFamilyIndices.end());
                
            std::uint32_t minImageCount =
            surfaceCapabilities.maxImageCount > 0 &&
            surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount ?
            surfaceCapabilities.maxImageCount : surfaceCapabilities.minImageCount + 1;
            
            VkSwapchainCreateInfoKHR swapchainCreateInfo{};
            swapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            swapchainCreateInfo.pNext                    = nullptr;
            swapchainCreateInfo.flags                    = 0;
            swapchainCreateInfo.surface                  = surface.get();
            swapchainCreateInfo.minImageCount            = minImageCount;
            swapchainCreateInfo.imageFormat              = surfaceFormat.format;
            swapchainCreateInfo.imageColorSpace          = surfaceFormat.colorSpace;
            swapchainCreateInfo.imageExtent              = extent;
            swapchainCreateInfo.imageArrayLayers         = 1;
            swapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            swapchainCreateInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount    = queueFamilyIndices.size() > 1 ? static_cast<std::uint32_t>(queueFamilyIndices.size()) : 0;
            swapchainCreateInfo.pQueueFamilyIndices      = queueFamilyIndices.size() > 1 ? queueFamilyIndices.data() : nullptr;
            swapchainCreateInfo.preTransform             = surfaceCapabilities.currentTransform;
            swapchainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            swapchainCreateInfo.presentMode              = presentMode;
            swapchainCreateInfo.clipped                  = VK_TRUE;
            swapchainCreateInfo.oldSwapchain             = VK_NULL_HANDLE;
            
            if (vkCreateSwapchainKHR(this->device_.get(), &swapchainCreateInfo, nullptr, &this->swapchain_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create mgo::vk::Swapchain!");
            MGO_LOG("Created mgo::vk::Swapchain!");
        }
        
        Swapchain::~Swapchain() noexcept
        {
            vkDestroySwapchainKHR(this->device_.get(), this->swapchain_, nullptr);
            MGO_LOG("Destroyed mgo::vk::Swapchain!");
        }
        
        VkSwapchainKHR Swapchain::get() const noexcept
        {
            return this->swapchain_;
        }
        
#pragma mark - mgo::vk::ImageViews
        ImageViews::ImageViews(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device, const Swapchain& swapchain)
        :
        surface_(surface),
        physicalDevice_(physicalDevice),
        device_(device),
        swapchain_(swapchain)
        {
            std::uint32_t imageCount;
            vkGetSwapchainImagesKHR(this->device_.get(), this->swapchain_.get(), &imageCount, nullptr);
            
            this->imageViews_.resize(static_cast<std::size_t>(imageCount));
            this->images_.resize(static_cast<std::size_t>(imageCount));
            vkGetSwapchainImagesKHR(this->device_.get(), this->swapchain_.get(), &imageCount, this->images_.data());
        
            for (std::size_t i = 0; i < static_cast<std::size_t>(imageCount); i++)
            {
                VkImageViewCreateInfo imageViewCreateInfo = this->getVkImageViewCreateInfo(this->images_[i]);
                if (vkCreateImageView(this->device_.get(), &imageViewCreateInfo, nullptr, &this->imageViews_[i]) != VK_SUCCESS)
                    throw std::runtime_error("Failed to create mgo::vk::ImageViews!");
            }
            MGO_LOG("Created mgo::vk::ImageViews!");
        }
        
        ImageViews::~ImageViews() noexcept
        {
            for (auto imageView : this->imageViews_)
                vkDestroyImageView(this->device_.get(), imageView, nullptr);
        }
        
        std::vector<VkImageView> ImageViews::get() const noexcept
        {
            return this->imageViews_;
        }
        
        VkImageViewCreateInfo ImageViews::getVkImageViewCreateInfo(VkImage image) const noexcept
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext                            = nullptr;
            imageViewCreateInfo.flags                            = 0;
            imageViewCreateInfo.image                            = image;
            imageViewCreateInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format                           = this->surface_.getVkSurfaceFormatKHR(this->physicalDevice_).format;
            imageViewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel    = 0;
            imageViewCreateInfo.subresourceRange.levelCount      = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer  = 0;
            imageViewCreateInfo.subresourceRange.layerCount      = 1;
            return imageViewCreateInfo;
        }

#pragma mark - mgo::vk::RenderPass
        RenderPass::RenderPass(const Surface& surface, const PhysicalDevice& physicalDevice, const Device& device)
        :
        surface_(surface),
        physicalDevice_(physicalDevice),
        device_(device)
        {
            std::vector<VkAttachmentDescription> attachmentDescriptions = {this->getVkAttachmentDescription()};
            VkAttachmentReference attachmentReference = this->getVkAttachmentReference();
            std::vector<VkSubpassDescription> subpassDescriptions = {this->getVkSubpassDescription(&attachmentReference)};

            VkRenderPassCreateInfo renderPassCreateInfo{};
            renderPassCreateInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassCreateInfo.pNext              = nullptr;
            renderPassCreateInfo.flags              = 0;
            renderPassCreateInfo.attachmentCount    = static_cast<std::uint32_t>(attachmentDescriptions.size());
            renderPassCreateInfo.pAttachments       = attachmentDescriptions.data();
            renderPassCreateInfo.subpassCount       = static_cast<std::uint32_t>(subpassDescriptions.size());
            renderPassCreateInfo.pSubpasses         = subpassDescriptions.data();
            renderPassCreateInfo.dependencyCount    = 0;
            renderPassCreateInfo.pDependencies      = nullptr;
            
            if (vkCreateRenderPass(this->device_.get(), &renderPassCreateInfo, nullptr, &this->renderPass_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create mgo::vk::RenderPass!");
            MGO_LOG("Created mgo::vk::RenderPass!");
        }
        
        RenderPass::~RenderPass() noexcept
        {
            vkDestroyRenderPass(this->device_.get(), this->renderPass_, nullptr);
            MGO_LOG("Destroyed mgo::vk::RenderPass!");
        }
        
        VkRenderPass RenderPass::get() const noexcept
        {
            return this->renderPass_;
        }
        
        VkAttachmentDescription RenderPass::getVkAttachmentDescription() const noexcept
        {
            VkAttachmentDescription attachmentDescription{};
            attachmentDescription.flags           = 0;
            attachmentDescription.format          = this->surface_.getVkSurfaceFormatKHR(this->physicalDevice_).format;;
            attachmentDescription.samples         = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescription.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescription.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescription.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescription.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachmentDescription.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescription.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            return attachmentDescription;
        }
        
        VkAttachmentReference RenderPass::getVkAttachmentReference() const noexcept
        {
            VkAttachmentReference attachmentReference;
            attachmentReference.attachment  = 0;
            attachmentReference.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            return attachmentReference;
        }
        
        VkSubpassDescription RenderPass::getVkSubpassDescription(const VkAttachmentReference* pColorAttachmentReference) const noexcept
        {
            VkSubpassDescription subpassDescription{};
            subpassDescription.flags                    = 0;
            subpassDescription.pipelineBindPoint        = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescription.inputAttachmentCount     = 0;
            subpassDescription.pInputAttachments        = nullptr;
            subpassDescription.colorAttachmentCount     = 1;
            subpassDescription.pColorAttachments        = pColorAttachmentReference;
            subpassDescription.pResolveAttachments      = nullptr;
            subpassDescription.pDepthStencilAttachment  = nullptr;
            subpassDescription.preserveAttachmentCount  = 0;
            subpassDescription.pPreserveAttachments     = nullptr;
            return subpassDescription;
        }
        
#pragma mark - mgo::vk::PipelineLayout
        PipelineLayout::PipelineLayout(const Device& device)
        :
        device_(device)
        {
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.pNext                   = nullptr;
            pipelineLayoutCreateInfo.flags                   = 0;
            pipelineLayoutCreateInfo.setLayoutCount          = 0;
            pipelineLayoutCreateInfo.pSetLayouts             = nullptr;
            pipelineLayoutCreateInfo.pushConstantRangeCount  = 0;
            pipelineLayoutCreateInfo.pPushConstantRanges     = nullptr;
            
            if (vkCreatePipelineLayout(this->device_.get(), &pipelineLayoutCreateInfo, nullptr, &this->pipelineLayout_) != VK_SUCCESS)
                throw std::runtime_error("Failed to create mgo::vk::PipelineLayout!");
            MGO_LOG("Created mgo::vk::PipelineLayout!");
        }
            
        PipelineLayout::~PipelineLayout() noexcept
        {
            vkDestroyPipelineLayout(this->device_.get(), this->pipelineLayout_, nullptr);
            MGO_LOG("Destroyed mgo::vk::PipelineLayout!");
        }
            
        VkPipelineLayout PipelineLayout::get() const noexcept
        {
            return pipelineLayout_;
        }
        
#pragma mark - mgo::vk::Pipeline::ShaderModule
        Pipeline::ShaderModule::ShaderModule(const std::string& path, const Device& device)
        :
        device_(device)
        {
            std::ifstream fileStream(path, std::ios::ate | std::ios::binary);
            
            if (!fileStream.is_open())
                throw std::runtime_error("Failed to open shader: " + path);
            
            std::size_t fileSize = static_cast<std::size_t>(fileStream.tellg());
            std::vector<char> code(fileSize);
            
            fileStream.seekg(0);
            fileStream.read(code.data(), fileSize);
            fileStream.close();
            
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.pNext    = nullptr;
            shaderModuleCreateInfo.flags    = 0;
            shaderModuleCreateInfo.codeSize = code.size();
            shaderModuleCreateInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());
            
            if (vkCreateShaderModule(this->device_.get(), &shaderModuleCreateInfo, nullptr, &this->shaderModule_) != VK_SUCCESS)
                throw std::runtime_error("failed to create mgo::vk::Pipeline::ShaderModule!");
            MGO_LOG("Created mgo::vk::Pipeline::ShaderModule!");
        }
                
        Pipeline::ShaderModule::~ShaderModule() noexcept
        {
            vkDestroyShaderModule(this->device_.get(), this->shaderModule_, nullptr);
            MGO_LOG("Destroyed mgo::vk::Pipeline::ShaderModule!");
        }
               
        VkShaderModule Pipeline::ShaderModule::get() const noexcept
        {
            return this->shaderModule_;
        }
                
#pragma mark - mgo::vk::Pipeline
        Pipeline::Pipeline(const Device& device, const RenderPass& renderPass, const PipelineLayout& pipelineLayout)
        :
        device_(device),
        renderPass_(renderPass),
        pipelineLayout_(pipelineLayout)
        {
            ShaderModule vertShaderModule("MangosEngine/Vulkan/SPIR-V/vert.spv", this->device_);
            VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo =
            this->getVkPipelineShaderStageCreateInfo(vertShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
            
            ShaderModule fragShaderModule("MangosEngine/Vulkan/SPIR-V/frag.spv", this->device_);
            VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo =
            this->getVkPipelineShaderStageCreateInfo(fragShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
        
            std::vector<VkPipelineShaderStageCreateInfo> stages = {vertPipelineShaderStageCreateInfo, fragPipelineShaderStageCreateInfo};
            
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo =
            this->getVkPipelineVertexInputStateCreateInfo();
            
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo =
            this->getVkPipelineInputAssemblyStateCreateInfo();
            
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo =
            this->getVkPipelineViewportStateCreateInfo();
            
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo =
            this->getVkPipelineRasterizationStateCreateInfo();
            
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo =
            this->getVkPipelineMultisampleStateCreateInfo();

            std::vector<VkPipelineColorBlendAttachmentState> attachments = {this->getVkPipelineColorBlendAttachmentState()};
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo =
            this->getVkPipelineColorBlendStateCreateInfo(attachments);

            std::vector<VkDynamicState> dynamicStates ={VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo =
            this->getVkPipelineDynamicStateCreateInfo(dynamicStates);


            VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
            graphicsPipelineCreateInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            graphicsPipelineCreateInfo.stageCount           = static_cast<std::uint32_t>(stages.size());
            graphicsPipelineCreateInfo.pStages              = stages.data();
            graphicsPipelineCreateInfo.pVertexInputState    = &pipelineVertexInputStateCreateInfo;
            graphicsPipelineCreateInfo.pInputAssemblyState  = &pipelineInputAssemblyStateCreateInfo;
            graphicsPipelineCreateInfo.pViewportState       = &pipelineViewportStateCreateInfo;
            graphicsPipelineCreateInfo.pRasterizationState  = &pipelineRasterizationStateCreateInfo;
            graphicsPipelineCreateInfo.pMultisampleState    = &pipelineMultisampleStateCreateInfo;
            graphicsPipelineCreateInfo.pColorBlendState     = &pipelineColorBlendStateCreateInfo;
            graphicsPipelineCreateInfo.pDynamicState        = &pipelineDynamicStateCreateInfo;
            graphicsPipelineCreateInfo.layout               = this->pipelineLayout_.get();
            graphicsPipelineCreateInfo.renderPass           = this->renderPass_.get();
            graphicsPipelineCreateInfo.subpass              = 0;
            graphicsPipelineCreateInfo.basePipelineHandle   = VK_NULL_HANDLE;

            if (vkCreateGraphicsPipelines(this->device_.get(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &this->pipeline_) != VK_SUCCESS)
                throw std::runtime_error("failed to create mgo::vk::Pipeline!");
            MGO_LOG("Created mgo::vk::Pipeline!");
        }
        
        Pipeline::~Pipeline() noexcept
        {
            vkDestroyPipeline(this->device_.get(), this->pipeline_, nullptr);
            MGO_LOG("Destroyed mgo::vk::Pipeline!");
        }
        
        VkPipeline Pipeline::get() const noexcept
        {
            return this->pipeline_;
        }
        
        VkPipelineShaderStageCreateInfo Pipeline::getVkPipelineShaderStageCreateInfo(const ShaderModule& shaderModule,
                                                                                     VkShaderStageFlagBits stage) const noexcept
        {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{};
            pipelineShaderStageCreateInfo.sType                   = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            pipelineShaderStageCreateInfo.pNext                   = nullptr;
            pipelineShaderStageCreateInfo.flags                   = 0;
            pipelineShaderStageCreateInfo.stage                   = stage;
            pipelineShaderStageCreateInfo.module                  = shaderModule.get();
            pipelineShaderStageCreateInfo.pName                   = "main";
            pipelineShaderStageCreateInfo.pSpecializationInfo     = nullptr;
            return pipelineShaderStageCreateInfo;
        }
        
        VkPipelineVertexInputStateCreateInfo Pipeline::getVkPipelineVertexInputStateCreateInfo() const noexcept
        {
            VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
            pipelineVertexInputStateCreateInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            pipelineVertexInputStateCreateInfo.pNext                           = nullptr;
            pipelineVertexInputStateCreateInfo.flags                           = 0;
            pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount   = 0;
            pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions      = nullptr;
            pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
            pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions    = nullptr;
            return pipelineVertexInputStateCreateInfo;
        }
        
        VkPipelineInputAssemblyStateCreateInfo Pipeline::getVkPipelineInputAssemblyStateCreateInfo() const noexcept
        {
            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
            pipelineInputAssemblyStateCreateInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.pNext                    = nullptr;
            pipelineInputAssemblyStateCreateInfo.flags                    = 0;
            pipelineInputAssemblyStateCreateInfo.topology                 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable   = VK_FALSE;
            return pipelineInputAssemblyStateCreateInfo;
        }
        
        VkPipelineViewportStateCreateInfo Pipeline::getVkPipelineViewportStateCreateInfo() const noexcept
        {
            VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
            pipelineViewportStateCreateInfo.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            pipelineViewportStateCreateInfo.pNext         = nullptr;
            pipelineViewportStateCreateInfo.flags         = 0;
            pipelineViewportStateCreateInfo.viewportCount = 1;
            pipelineViewportStateCreateInfo.pViewports    = nullptr;
            pipelineViewportStateCreateInfo.scissorCount  = 1;
            pipelineViewportStateCreateInfo.pScissors     = nullptr;
            return pipelineViewportStateCreateInfo;
        }
        
        VkPipelineRasterizationStateCreateInfo Pipeline::getVkPipelineRasterizationStateCreateInfo() const noexcept
        {
            VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
            pipelineRasterizationStateCreateInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            pipelineRasterizationStateCreateInfo.pNext                    = nullptr;
            pipelineRasterizationStateCreateInfo.flags                    = 0;
            pipelineRasterizationStateCreateInfo.depthClampEnable         = VK_FALSE;
            pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable  = VK_FALSE;
            pipelineRasterizationStateCreateInfo.polygonMode              = VK_POLYGON_MODE_FILL;
            pipelineRasterizationStateCreateInfo.cullMode                 = VK_CULL_MODE_BACK_BIT;
            pipelineRasterizationStateCreateInfo.frontFace                = VK_FRONT_FACE_CLOCKWISE;
            pipelineRasterizationStateCreateInfo.depthBiasEnable          = VK_FALSE;
            pipelineRasterizationStateCreateInfo.depthBiasConstantFactor  = 0.0f;
            pipelineRasterizationStateCreateInfo.depthBiasClamp           = 0.0f;
            pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor     = 0.0f;
            pipelineRasterizationStateCreateInfo.lineWidth                = 1.0f;
            return pipelineRasterizationStateCreateInfo;
        }
        
        VkPipelineMultisampleStateCreateInfo Pipeline::getVkPipelineMultisampleStateCreateInfo() const noexcept
        {
            VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
            pipelineMultisampleStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            pipelineMultisampleStateCreateInfo.pNext                  = nullptr;
            pipelineMultisampleStateCreateInfo.flags                  = 0;
            pipelineMultisampleStateCreateInfo.rasterizationSamples   = VK_SAMPLE_COUNT_1_BIT;
            pipelineMultisampleStateCreateInfo.sampleShadingEnable    = VK_FALSE;
            pipelineMultisampleStateCreateInfo.minSampleShading       = 0.0f;
            pipelineMultisampleStateCreateInfo.pSampleMask            = nullptr;
            pipelineMultisampleStateCreateInfo.alphaToCoverageEnable  = VK_FALSE;
            pipelineMultisampleStateCreateInfo.alphaToOneEnable       = VK_FALSE;
            return pipelineMultisampleStateCreateInfo;
        }
        
        VkPipelineColorBlendAttachmentState Pipeline::getVkPipelineColorBlendAttachmentState() const noexcept
        {
            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
            pipelineColorBlendAttachmentState.blendEnable            = VK_FALSE;
            pipelineColorBlendAttachmentState.srcColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.colorBlendOp           = VK_BLEND_OP_ADD;
            pipelineColorBlendAttachmentState.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO;
            pipelineColorBlendAttachmentState.alphaBlendOp           = VK_BLEND_OP_ADD;
            pipelineColorBlendAttachmentState.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT;
            pipelineColorBlendAttachmentState.colorWriteMask        |= VK_COLOR_COMPONENT_G_BIT;
            pipelineColorBlendAttachmentState.colorWriteMask        |= VK_COLOR_COMPONENT_B_BIT;
            pipelineColorBlendAttachmentState.colorWriteMask        |= VK_COLOR_COMPONENT_A_BIT;
            return pipelineColorBlendAttachmentState;
        }

        VkPipelineColorBlendStateCreateInfo
        Pipeline::getVkPipelineColorBlendStateCreateInfo(const std::vector<VkPipelineColorBlendAttachmentState>& attachments) const noexcept
        {
            VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
            pipelineColorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            pipelineColorBlendStateCreateInfo.pNext             = nullptr;
            pipelineColorBlendStateCreateInfo.flags             = 0;
            pipelineColorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
            pipelineColorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_COPY;
            pipelineColorBlendStateCreateInfo.attachmentCount   = static_cast<std::uint32_t>(attachments.size());
            pipelineColorBlendStateCreateInfo.pAttachments      = attachments.data();
            pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
            pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
            pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
            pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
            return pipelineColorBlendStateCreateInfo;
        }

        VkPipelineDynamicStateCreateInfo Pipeline::getVkPipelineDynamicStateCreateInfo(const std::vector<VkDynamicState>& dynamicStates) const noexcept
        {
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
            pipelineDynamicStateCreateInfo.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            pipelineDynamicStateCreateInfo.pNext              = nullptr;
            pipelineDynamicStateCreateInfo.flags              = 0;
            pipelineDynamicStateCreateInfo.dynamicStateCount  = static_cast<std::uint32_t>(dynamicStates.size());
            pipelineDynamicStateCreateInfo.pDynamicStates     = dynamicStates.data();
            return pipelineDynamicStateCreateInfo;
        }
    }
}
