#include "app.hpp"

namespace mgo
{
#pragma mark - App::RenderWindow
    
    App::RenderWindow::RenderWindow(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight)
    :
    windowName_(windowName), windowHight_(windowHight), windowWidth_(windowWidth), pWindow_(nullptr)
    {
        glfwSetErrorCallback(this->errorCallback);
        
        if (!glfwInit())
            throw std::runtime_error("Failed to init GLFW!");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        this->pWindow_ = glfwCreateWindow(this->windowWidth_, this->windowHight_, this->windowName_.c_str(), nullptr, nullptr);
        
        if (!this->pWindow_)
            throw std::runtime_error("Failed to create GLFWwindow!");
    }
    
    App::RenderWindow::~RenderWindow() noexcept
    {
        glfwDestroyWindow(this->pWindow_);
        glfwTerminate();
    }
    
    VkResult App::RenderWindow::createSurface(VkInstance instance, VkSurfaceKHR* pSurface) const
    {
        return glfwCreateWindowSurface(instance, this->pWindow_, nullptr, pSurface);
    }
    
    VkExtent2D App::RenderWindow::GetFramebufferSize() const noexcept
    {
        int width, height;
        
        glfwGetFramebufferSize(this->pWindow_, &width, &height);
        
        return {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    }
    
    bool App::RenderWindow::shouldClose() const noexcept
    {
        return glfwWindowShouldClose(this->pWindow_);
    }
    
    void App::RenderWindow::pollEvents() noexcept
    {
        glfwPollEvents();
    }
    
    void App::RenderWindow::errorCallback(int error, const char* description)
    {
        std::cerr << "GLFW error: " << description << std::endl;
    }
    
#pragma mark - App::Device
    App::Device::Device(const std::string& appName, const RenderWindow& window)
    :
    appName_(appName),
    engineName_("Magnos Engine"),
    validationLayers_(this->getValidationLayers()),
    instanceExtensions_(this->getinstanceExtensions()),
    deviceExtensions_(this->getdeviceExtensions())
    {
        this->createVulkanInstance();
#ifdef DEBUG
        this->createDebugMessenger();
#endif
        this->createSurface(window);
        this->createDevice();
        this->createSwapchain(window);
        this->createImageViews();
        this->createRenderPass();
        this->createGraphicsPipeline();
    }
    
    App::Device::~Device()
    {
        this->destroyGraphicsPipeline();
        this->destroyRenderPass();
        this->destroyImageViews();
        this->destroySwapchain();
        this->destroyDevice();
        this->destroySurface();
#ifdef DEBUG
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
        
#ifdef __APPLE__
        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
#ifdef DEBUG
        requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
        return requiredExtensions;
    }
    
    std::vector<const char*> App::Device::getdeviceExtensions() noexcept
    {
#ifdef __APPLE__
        return std::vector<const char*>({VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_NV_GLSL_SHADER_EXTENSION_NAME, "VK_KHR_portability_subset"});
#else
        return std::vector<const char*>({VK_KHR_SWAPCHAIN_EXTENSION_NAME});
#endif
    }

    void App::Device::createVulkanInstance()
    {
#ifdef DEBUG
        this->checkValidationLayerSupport();
        this->checkInstanceExtensionSupport();
#endif
        VkApplicationInfo appInfo{};
        this->populateApplicationInfo(appInfo);
        
        VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo{};
        this->populateDebugUtilsMessengerCreateInfoEXT(DebugCreateInfo);
        
        VkInstanceCreateInfo createInfo{};
        this->populateInstanceCreateInfo(createInfo, &appInfo, &DebugCreateInfo);
        
        if (vkCreateInstance(&createInfo, nullptr, &this->instance_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance!");
    }
     
    void App::Device::createDebugMessenger()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        this->populateDebugUtilsMessengerCreateInfoEXT(createInfo);
        
        
        if (vkCreateDebugUtilsMessengerEXT(this->instance_, &createInfo, nullptr, &this->debugMessenger_) != VK_SUCCESS)
            throw std::runtime_error("Failed to set up debug messenger!");
    }
    
    void App::Device::createSurface(const RenderWindow& window)
    {
        if (window.createSurface(this->instance_, &this->surface_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create window surface");
    }
    
    void App::Device::createDevice()
    {
        std::uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->instance_, &deviceCount, nullptr);
        
        if (deviceCount == 0)
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        
        std::vector<VkPhysicalDevice> phyicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(this->instance_, &deviceCount, phyicalDevices.data());
        
        std::multimap<std::uint8_t, VkPhysicalDevice, std::greater<std::uint8_t>> phyicalDevicesCandidates;
        
        for (auto potentialPhyicalDevice : phyicalDevices)
            phyicalDevicesCandidates.emplace(this->rankPhysicalDevices(potentialPhyicalDevice), potentialPhyicalDevice);
        
        if (phyicalDevicesCandidates.begin()->first == 0)
            throw std::runtime_error("Failed to find a suitable GPU!");
        
        this->physicalDevice_ = phyicalDevicesCandidates.begin()->second;
        
        this->populateQueueFamilyIndices(this->indices_, this->physicalDevice_, this->surface_, 1.0f);
        
        if (!this->indices_.graphicsFamily_.has_value() && !this->indices_.presentFamily_.has_value())
            throw std::runtime_error("failed to find queue family!");
        
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {this->indices_.graphicsFamily_.value(), this->indices_.presentFamily_.value()};
        
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            this->populateDeviceQueueCreateInfo(queueCreateInfo, queueFamily);
            queueCreateInfos.push_back(queueCreateInfo);
        }
        
        VkPhysicalDeviceFeatures physicalDeviceFeatures;
        this->populatePhysicalDeviceFeatures(physicalDeviceFeatures);
        
        VkDeviceCreateInfo deviceCreateInfo;
        this->populateDeviceCreateInfo(deviceCreateInfo, queueCreateInfos, &physicalDeviceFeatures);
        
        if (vkCreateDevice(this->physicalDevice_, &deviceCreateInfo, nullptr, &this->device_) != VK_SUCCESS)
            throw std::runtime_error("failed to create device!");
        
        vkGetDeviceQueue(this->device_, this->indices_.graphicsFamily_.value(), 0, &this->graphicsQueue_);
        vkGetDeviceQueue(this->device_, this->indices_.presentFamily_.value(), 0, &this->presentQueue_);
    }
    
    void App::Device::createSwapchain(const RenderWindow& window)
    {
        this->setupSurfaceCapabilities();
        this->setupSurfaceFormat();
        this->setupPresentMode();
        this->setupExtent(window);
        
        VkSwapchainCreateInfoKHR swapchainCreateInfo;
        this->populateSwapchainCreateInfoKHR(swapchainCreateInfo);
        
        if (vkCreateSwapchainKHR(this->device_, &swapchainCreateInfo, nullptr, &this->swapchain_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain!");
    }
    
    void App::Device::createImageViews()
    {
        std::uint32_t imageCount;
        vkGetSwapchainImagesKHR(this->device_, this->swapchain_, &imageCount, nullptr);
        
        this->images_.resize(static_cast<std::size_t>(imageCount));
        vkGetSwapchainImagesKHR(this->device_, this->swapchain_, &imageCount, this->images_.data());
    
        this->imageViews_.resize(static_cast<std::size_t>(imageCount));
        
        for (std::size_t i = 0; i < static_cast<std::size_t>(imageCount); i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo;
            populateImageViewCreateInfo(imageViewCreateInfo, this->images_[i]);
                                        
            if (vkCreateImageView(this->device_, &imageViewCreateInfo, nullptr, &this->imageViews_[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create image views!");
        }
    }
    
    void App::Device::createRenderPass()
    {
        VkAttachmentDescription colourDescription;
        this->populateAttachmentDescription(colourDescription);
        std::vector<VkAttachmentDescription> colourDescriptions = {colourDescription};
        
        VkAttachmentReference colourAttachment;
        this->populateAttachmentReference(colourAttachment);
        
        VkSubpassDescription subpassDescription;
        this->populateSubpassDescription(subpassDescription, &colourAttachment);
        std::vector<VkSubpassDescription> subpassDescriptions = {subpassDescription};

        VkRenderPassCreateInfo renderPassInfo;
        this->populateRenderPassCreateInfo(renderPassInfo, colourDescriptions, subpassDescriptions);
        
        if (vkCreateRenderPass(this->device_, &renderPassInfo, nullptr, &this->renderPass_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create render pass!");

    }
    
    void App::Device::createGraphicsPipeline()
    {
        /*
        VkShaderModuleCreateInfo vertShaderModuleCreateInfo;
        this->populateShaderModuleCreateInfo(vertShaderModuleCreateInfo, this->readFile("MangosEngine/shaders/vert.spv"));
        
        VkShaderModule vertShaderModule;
        if (vkCreateShaderModule(this->device_, &vertShaderModuleCreateInfo, nullptr, &vertShaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create vertex shader module!");
        
        
        VkPipelineShaderStageCreateInfo vertPipelineShaderStageCreateInfo;
        this->populatePipelineShaderStageCreateInfo(vertPipelineShaderStageCreateInfo,
                                                    VK_SHADER_STAGE_VERTEX_BIT,
                                                    vertShaderModule);
        
        VkShaderModuleCreateInfo fragShaderModuleCreateInfo;
        this->populateShaderModuleCreateInfo(fragShaderModuleCreateInfo, this->readFile("MangosEngine/shaders/frag.spv"));
        
        VkShaderModule fragShaderModule;
        if (vkCreateShaderModule(this->device_, &fragShaderModuleCreateInfo, nullptr, &fragShaderModule) != VK_SUCCESS)
            throw std::runtime_error("Failed to create fragment shader module!");
        
        
        VkPipelineShaderStageCreateInfo fragPipelineShaderStageCreateInfo;
        this->populatePipelineShaderStageCreateInfo(fragPipelineShaderStageCreateInfo,
                                                    VK_SHADER_STAGE_FRAGMENT_BIT,
                                                    fragShaderModule);
        
    
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {vertPipelineShaderStageCreateInfo, fragPipelineShaderStageCreateInfo};
        
        std::vector<VkDynamicState> dynamicStates =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo;
        this->populatePipelineDynamicStateCreateInfo(pipelineDynamicStateCreateInfo, dynamicStates);
        
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
        this->populatePipelineLayoutCreateInfo(pipelineLayoutCreateInfo);
        
        if (vkCreatePipelineLayout(this->device_, &pipelineLayoutCreateInfo, nullptr, &this->pipelineLayout_) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout!");
        
        
        
        
        
        VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo;
        this->populatePipelineVertexInputStateCreateInfo(pipelineVertexInputStateCreateInfo);
        
        VkPipelineInputAssemblyStateCreateInfo populatePipelineInputAssemblyStateCreateInfo;
        this->populatePipelineInputAssemblyStateCreateInfo(populatePipelineInputAssemblyStateCreateInfo);
        
        VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo;
        this->populatePipelineInputAssemblyStateCreateInfo(pipelineInputAssemblyStateCreateInfo);
        
        VkViewport viewport;
        this->populateViewport(viewport);
        VkRect2D scissor;
        this->populateRect2D(scissor);
        VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo;
        this->populatePipelineViewportStateCreateInfo(pipelineViewportStateCreateInfo, &viewport, &scissor);
        
        VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo;
        this->populatePipelineRasterizationStateCreateInfo(pipelineRasterizationStateCreateInfo);
        
        VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo;
        this->populatePipelineMultisampleStateCreateInfo(pipelineMultisampleStateCreateInfo);
        
        VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo;
        this->populatePipelineDepthStencilStateCreateInfo(pipelineDepthStencilStateCreateInfo);
        
        VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState;
        this->populatePipelineColorBlendAttachmentState(pipelineColorBlendAttachmentState);
        VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo;
        this->populatePipelineColorBlendStateCreateInfo(pipelineColorBlendStateCreateInfo, &pipelineColorBlendAttachmentState);
        
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo;
        this->populateGraphicsPipelineCreateInfo(graphicsPipelineCreateInfo,
                                                 shaderStages,
                                                 &pipelineVertexInputStateCreateInfo,
                                                 &pipelineInputAssemblyStateCreateInfo,
                                                 nullptr,
                                                 &pipelineViewportStateCreateInfo,
                                                 &pipelineRasterizationStateCreateInfo,
                                                 &pipelineMultisampleStateCreateInfo,
                                                 &pipelineDepthStencilStateCreateInfo,
                                                 &pipelineColorBlendStateCreateInfo,
                                                 &pipelineDynamicStateCreateInfo);
        
        
        vkDestroyShaderModule(this->device_, vertShaderModule, nullptr);
        vkDestroyShaderModule(this->device_, fragShaderModule, nullptr);
        
        
        -------------------------------------------------------------------------------- */
        
        auto vertShaderCode = readFile("MangosEngine/shaders/vert.spv");
        auto fragShaderCode = readFile("MangosEngine/shaders/frag.spv");

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };
        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout_;
        pipelineInfo.renderPass = renderPass_;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        vkDestroyShaderModule(device_, fragShaderModule, nullptr);
        vkDestroyShaderModule(device_, vertShaderModule, nullptr);

    }

    VkShaderModule App::Device::createShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }
    /* -------------------------------------------------------------------------------- */


    void App::Device::destoryVulkanInstance() noexcept
    {
        vkDestroyInstance(this->instance_, nullptr);
    }
    
    void App::Device::destoryDebugMessenger() noexcept
    {
        this->DestroyDebugUtilsMessengerEXT(this->instance_, this->debugMessenger_, nullptr);
    }
    
    void App::Device::destroySurface() noexcept
    {
        vkDestroySurfaceKHR(this->instance_, this->surface_, nullptr);
    }
    
    void App::Device::destroyDevice() noexcept
    {
        vkDestroyDevice(this->device_, nullptr);
    }
    
    void App::Device::destroySwapchain() noexcept
    {
        vkDestroySwapchainKHR(this->device_, this->swapchain_, nullptr);
    }
    
    void App::Device::destroyImageViews() noexcept
    {
        for (auto imageView : this->imageViews_)
            vkDestroyImageView(this->device_, imageView, nullptr);
    }
    
    void App::Device::destroyGraphicsPipeline() noexcept
    {
        vkDestroyPipelineLayout(this->device_, this->pipelineLayout_, nullptr);
        vkDestroyPipeline(this->device_, this->pipeline_, nullptr);
    }
    
    void App::Device::destroyRenderPass() noexcept
    {
        vkDestroyRenderPass(this->device_, this->renderPass_, nullptr);
    }

    std::uint8_t App::Device::rankPhysicalDevices(VkPhysicalDevice physicalDevice) const noexcept
    {
        std::uint8_t value = 0;
        
        VkPhysicalDeviceProperties phyicalDevicesProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &phyicalDevicesProperties);
        
        VkPhysicalDeviceFeatures phyicalDevicesFeatures;
        vkGetPhysicalDeviceFeatures(physicalDevice, &phyicalDevicesFeatures);
        
        QueueFamilyIndices indices;
        this->populateQueueFamilyIndices(indices, physicalDevice, this->surface_, 1.0f);
        
        std::uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, this->surface_, &formatCount, nullptr);
        
        std::uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, this->surface_, &presentModeCount, nullptr);
        
        switch (phyicalDevicesProperties.deviceType)
        {
            case (VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)   : {value = 2; break;}
            case (VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)     : {value = 1; break;}
            default : return 0;
        };
        
        if (!indices.graphicsFamily_.has_value() || !indices.presentFamily_.has_value())
            return 0;
        
        if (!this->checkDeviceExtensionSupport(physicalDevice))
            return 0;
        
        if (presentModeCount == 0 || formatCount == 0)
            return 0;
        
        return value;
    }
    
    void App::Device::populateApplicationInfo(VkApplicationInfo& appInfo) const noexcept
    {
        appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pNext               = nullptr;
        appInfo.pApplicationName    = this->engineName_.c_str();
        appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName         = this->appName_.c_str();
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
#ifdef DEBUG
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = pDebugCreateInfo;
        instanceCreateInfo.pApplicationInfo         = pAppInfo;
        instanceCreateInfo.enabledLayerCount        = static_cast<std::uint32_t>(this->validationLayers_.size());
        instanceCreateInfo.ppEnabledLayerNames      = this->validationLayers_.data();
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->instanceExtensions_.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->instanceExtensions_.data();
#else
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext                    = nullptr;
        instanceCreateInfo.pApplicationInfo         = pAppInfo;
        instanceCreateInfo.enabledLayerCount        = 0;
        instanceCreateInfo.ppEnabledLayerNames      = nullptr;
        instanceCreateInfo.enabledExtensionCount    = static_cast<std::uint32_t>(this->instanceExtensions_.size());
        instanceCreateInfo.ppEnabledExtensionNames  = this->instanceExtensions_.data();
#endif
#ifdef __APPLE__
        instanceCreateInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
        instanceCreateInfo.flags = 0;
#endif
    }
    
    void App::Device::populateDeviceQueueCreateInfo(VkDeviceQueueCreateInfo& queueCreateInfo, std::uint32_t queueFamily) const noexcept
    {
        queueCreateInfo.sType               = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext               = nullptr;
        queueCreateInfo.flags               = 0;
        queueCreateInfo.queueFamilyIndex    = queueFamily;
        queueCreateInfo.queueCount          = 1;
        queueCreateInfo.pQueuePriorities    = &this->indices_.queuePriority_;
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
                                               const std::vector<VkDeviceQueueCreateInfo>& QueueCreateInfos,
                                               const VkPhysicalDeviceFeatures* pPhysicalDeviceFeatures) const noexcept
    {
#ifdef DEBUG
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = static_cast<std::uint32_t>(QueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos          = QueueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount          = static_cast<std::uint32_t>(this->validationLayers_.size());
        deviceCreateInfo.ppEnabledLayerNames        = this->validationLayers_.data();
        deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(this->deviceExtensions_.size());
        deviceCreateInfo.ppEnabledExtensionNames    = this->deviceExtensions_.data();
        deviceCreateInfo.pEnabledFeatures           = pPhysicalDeviceFeatures;
#else
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext                      = nullptr;
        deviceCreateInfo.flags                      = 0;
        deviceCreateInfo.queueCreateInfoCount       = static_cast<std::uint32_t>(QueueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos          = QueueCreateInfos.data();
        deviceCreateInfo.enabledLayerCount          = 0;
        deviceCreateInfo.ppEnabledLayerNames        = nullptr;
        deviceCreateInfo.enabledExtensionCount      = static_cast<std::uint32_t>(this->deviceExtensions_.size());
        deviceCreateInfo.ppEnabledExtensionNames    = this->deviceExtensions_.data();
        deviceCreateInfo.pEnabledFeatures           = pPhysicalDeviceFeatures;
#endif
    }
    
    void App::Device::populateQueueFamilyIndices(QueueFamilyIndices& indices, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, float queuePriority) const noexcept
    {
        indices.queuePriority_ = queuePriority;
        
        std::uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        
        std::uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily_ = i;
            
            if (presentSupport)
                indices.presentFamily_ = i;
            
            i++;
        }
    }
    
    void App::Device::populateSwapchainCreateInfoKHR(VkSwapchainCreateInfoKHR& SwapchainCreateInfo) const noexcept
    {
        std::uint32_t minImageCount =
        this->surfaceCapabilities_.maxImageCount > 0 && this->surfaceCapabilities_.minImageCount + 1 > this->surfaceCapabilities_.maxImageCount ?
        this->surfaceCapabilities_.maxImageCount : this->surfaceCapabilities_.minImageCount + 1;
        
        if (this->indices_.graphicsFamily_ != this->indices_.presentFamily_)
        {
            std::array<std::uint32_t, 2> queueFamilyIndices = {this->indices_.graphicsFamily_.value(), this->indices_.presentFamily_.value()};
            
            SwapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            SwapchainCreateInfo.pNext                    = nullptr;
            SwapchainCreateInfo.flags                    = 0;
            SwapchainCreateInfo.surface                  = this->surface_;
            SwapchainCreateInfo.minImageCount            = minImageCount;
            SwapchainCreateInfo.imageFormat              = this->surfaceFormat_.format;
            SwapchainCreateInfo.imageColorSpace          = this->surfaceFormat_.colorSpace;
            SwapchainCreateInfo.imageExtent              = this->extent_;
            SwapchainCreateInfo.imageArrayLayers         = 1;
            SwapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            SwapchainCreateInfo.imageSharingMode         = VK_SHARING_MODE_CONCURRENT;
            SwapchainCreateInfo.queueFamilyIndexCount    = 2;
            SwapchainCreateInfo.pQueueFamilyIndices      = queueFamilyIndices.data();
            SwapchainCreateInfo.preTransform             = this->surfaceCapabilities_.currentTransform;
            SwapchainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            SwapchainCreateInfo.presentMode              = this->presentMode_;
            SwapchainCreateInfo.clipped                  = VK_TRUE;
            SwapchainCreateInfo.oldSwapchain             = VK_NULL_HANDLE;
        }
        else
        {
            SwapchainCreateInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            SwapchainCreateInfo.pNext                    = nullptr;
            SwapchainCreateInfo.flags                    = 0;
            SwapchainCreateInfo.surface                  = this->surface_;
            SwapchainCreateInfo.minImageCount            = minImageCount;
            SwapchainCreateInfo.imageFormat              = this->surfaceFormat_.format;
            SwapchainCreateInfo.imageColorSpace          = this->surfaceFormat_.colorSpace;
            SwapchainCreateInfo.imageExtent              = this->extent_;
            SwapchainCreateInfo.imageArrayLayers         = 1;
            SwapchainCreateInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            SwapchainCreateInfo.imageSharingMode         = VK_SHARING_MODE_EXCLUSIVE;
            SwapchainCreateInfo.queueFamilyIndexCount    = 0;
            SwapchainCreateInfo.pQueueFamilyIndices      = nullptr;
            SwapchainCreateInfo.preTransform             = this->surfaceCapabilities_.currentTransform;
            SwapchainCreateInfo.compositeAlpha           = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            SwapchainCreateInfo.presentMode              = this->presentMode_;
            SwapchainCreateInfo.clipped                  = VK_TRUE;
            SwapchainCreateInfo.oldSwapchain             = VK_NULL_HANDLE;
        }
    }
    
    void App::Device::populateImageViewCreateInfo(VkImageViewCreateInfo& imageViewCreateInfo, VkImage image) const noexcept
    {
        imageViewCreateInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext                            = nullptr;
        imageViewCreateInfo.flags                            = 0;
        imageViewCreateInfo.image                            = image;
        imageViewCreateInfo.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format                           = this->surfaceFormat_.format;
        imageViewCreateInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel    = 0;
        imageViewCreateInfo.subresourceRange.levelCount      = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer  = 0;
        imageViewCreateInfo.subresourceRange.layerCount      = 1;
    }

    void App::Device::populateShaderModuleCreateInfo(VkShaderModuleCreateInfo& shaderModuleCreateInfo, const std::vector<char>& code) const noexcept
    {
        shaderModuleCreateInfo.sType       = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shaderModuleCreateInfo.pNext       = nullptr;
        shaderModuleCreateInfo.flags       = 0;
        shaderModuleCreateInfo.codeSize    = static_cast<std::uint32_t>(code.size());
        shaderModuleCreateInfo.pCode       = reinterpret_cast<const uint32_t*>(code.data());
    }
    
    void App::Device::populatePipelineShaderStageCreateInfo(VkPipelineShaderStageCreateInfo& pipelineShaderStageCreateInfo,
                                                            VkShaderStageFlagBits shaderStageFlagBits,
                                                            VkShaderModule shaderModule) const noexcept
    {
        pipelineShaderStageCreateInfo.sType                 = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.pNext                 = nullptr;
        pipelineShaderStageCreateInfo.flags                 = 0;
        pipelineShaderStageCreateInfo.stage                 = shaderStageFlagBits;
        pipelineShaderStageCreateInfo.module                = shaderModule;
        pipelineShaderStageCreateInfo.pName                 = "main";
        pipelineShaderStageCreateInfo.pSpecializationInfo   = nullptr;
    }

    void App::Device::populatePipelineDynamicStateCreateInfo(VkPipelineDynamicStateCreateInfo& pipelineDynamicStateCreateInfo,
                                                             const std::vector<VkDynamicState>& dynamicStates) const noexcept
    {
        pipelineDynamicStateCreateInfo.sType                = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.pNext                = nullptr;
        pipelineDynamicStateCreateInfo.flags                = 0;
        pipelineDynamicStateCreateInfo.dynamicStateCount    = static_cast<std::uint32_t>(dynamicStates.size());
        pipelineDynamicStateCreateInfo.pDynamicStates       = dynamicStates.data();
    }
    
    void App::Device::populatePipelineVertexInputStateCreateInfo(VkPipelineVertexInputStateCreateInfo& pipelineVertexInputStateCreateInfo) const noexcept
    {
        pipelineVertexInputStateCreateInfo.sType                            = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineVertexInputStateCreateInfo.pNext                            = nullptr;
        pipelineVertexInputStateCreateInfo.flags                            = 0;
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount    = 0;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions       = nullptr;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount  = 0;
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions     = nullptr;
    }
    
    void App::Device::populatePipelineInputAssemblyStateCreateInfo(VkPipelineInputAssemblyStateCreateInfo& pipelineInputAssemblyStateCreateInfo) const noexcept
    {
        pipelineInputAssemblyStateCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineInputAssemblyStateCreateInfo.pNext                  = nullptr;
        pipelineInputAssemblyStateCreateInfo.flags                  = 0;
        pipelineInputAssemblyStateCreateInfo.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    }

    void App::Device::populateViewport(VkViewport& viewport) const noexcept
    {
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>(this->extent_.width);
        viewport.height     = static_cast<float>(this->extent_.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;
    }
    
    void App::Device::populateRect2D(VkRect2D& rect2D) const noexcept
    {
        rect2D.offset = {0, 0};
        rect2D.extent = this->extent_;
    }
    
    void App::Device::populatePipelineViewportStateCreateInfo(VkPipelineViewportStateCreateInfo& pipelineViewportStateCreateInfo,
                                                              const VkViewport* pViewport, const VkRect2D* pScissor) const noexcept
    {
        pipelineViewportStateCreateInfo.sType           = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineViewportStateCreateInfo.pNext           = nullptr;
        pipelineViewportStateCreateInfo.flags           = 0;
        pipelineViewportStateCreateInfo.viewportCount   = 1;
        pipelineViewportStateCreateInfo.pViewports      = pViewport;
        pipelineViewportStateCreateInfo.scissorCount    = 1;
        pipelineViewportStateCreateInfo.pScissors       = pScissor;
    }
    
    void App::Device::populatePipelineRasterizationStateCreateInfo(VkPipelineRasterizationStateCreateInfo& pipelineRasterizationStateCreateInfo) const noexcept
    {
        pipelineRasterizationStateCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineRasterizationStateCreateInfo.pNext                      = nullptr;
        pipelineRasterizationStateCreateInfo.flags                      = 0;
        pipelineRasterizationStateCreateInfo.depthClampEnable           = VK_FALSE;
        pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable    = VK_FALSE;
        pipelineRasterizationStateCreateInfo.polygonMode                = VK_POLYGON_MODE_FILL;
        pipelineRasterizationStateCreateInfo.cullMode                   = VK_CULL_MODE_BACK_BIT;
        pipelineRasterizationStateCreateInfo.frontFace                  = VK_FRONT_FACE_CLOCKWISE;
        pipelineRasterizationStateCreateInfo.depthBiasEnable            = VK_FALSE;
        pipelineRasterizationStateCreateInfo.depthBiasConstantFactor    = 0.0f;
        pipelineRasterizationStateCreateInfo.depthBiasClamp             = 0.0f;
        pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor       = 0.0f;
        pipelineRasterizationStateCreateInfo.lineWidth                  = 1.0f;
    }
    
    void App::Device::populatePipelineMultisampleStateCreateInfo(VkPipelineMultisampleStateCreateInfo& pipelineMultisampleStateCreateInfo) const noexcept
    {
        pipelineMultisampleStateCreateInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineMultisampleStateCreateInfo.pNext                    = nullptr;
        pipelineMultisampleStateCreateInfo.flags                    = 0;
        pipelineMultisampleStateCreateInfo.rasterizationSamples     = VK_SAMPLE_COUNT_1_BIT;
        pipelineMultisampleStateCreateInfo.sampleShadingEnable      = VK_FALSE;
        pipelineMultisampleStateCreateInfo.minSampleShading         = 1.0f;
        pipelineMultisampleStateCreateInfo.pSampleMask              = nullptr;
        pipelineMultisampleStateCreateInfo.alphaToCoverageEnable    = VK_FALSE;
        pipelineMultisampleStateCreateInfo.alphaToOneEnable         = VK_FALSE;
    }
    
    void App::Device::populatePipelineDepthStencilStateCreateInfo(VkPipelineDepthStencilStateCreateInfo& pipelineDepthStencilStateCreateInfo) const noexcept
    {
        // needs impermentation!
    }
    
    void App::Device::populatePipelineColorBlendAttachmentState(VkPipelineColorBlendAttachmentState& pipelineColorBlendAttachmentState) const noexcept
    {
        pipelineColorBlendAttachmentState.blendEnable           = VK_TRUE;
        pipelineColorBlendAttachmentState.srcColorBlendFactor   = VK_BLEND_FACTOR_SRC_ALPHA;
        pipelineColorBlendAttachmentState.dstColorBlendFactor   = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        pipelineColorBlendAttachmentState.colorBlendOp          = VK_BLEND_OP_ADD;
        pipelineColorBlendAttachmentState.srcAlphaBlendFactor   = VK_BLEND_FACTOR_ONE;
        pipelineColorBlendAttachmentState.dstAlphaBlendFactor   = VK_BLEND_FACTOR_ZERO;
        pipelineColorBlendAttachmentState.alphaBlendOp          = VK_BLEND_OP_ADD;
        pipelineColorBlendAttachmentState.colorWriteMask        = VK_COLOR_COMPONENT_R_BIT |
                                                                  VK_COLOR_COMPONENT_G_BIT |
                                                                  VK_COLOR_COMPONENT_B_BIT |
                                                                  VK_COLOR_COMPONENT_A_BIT;
    }
    
    void App::Device::populatePipelineColorBlendStateCreateInfo(VkPipelineColorBlendStateCreateInfo& pipelineColorBlendStateCreateInfo,
                                                                const VkPipelineColorBlendAttachmentState* pPipelineColorBlendAttachmentState) const noexcept
    {
        pipelineColorBlendStateCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineColorBlendStateCreateInfo.pNext             = nullptr;
        pipelineColorBlendStateCreateInfo.flags             = 0;
        pipelineColorBlendStateCreateInfo.logicOpEnable     = VK_FALSE;
        pipelineColorBlendStateCreateInfo.logicOp           = VK_LOGIC_OP_COPY;
        pipelineColorBlendStateCreateInfo.attachmentCount   = 1;
        pipelineColorBlendStateCreateInfo.pAttachments      = pPipelineColorBlendAttachmentState;
        pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
    }

    void App::Device::populatePipelineLayoutCreateInfo(VkPipelineLayoutCreateInfo& pipelineLayoutCreateInfo) const noexcept
    {
        pipelineLayoutCreateInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext                  = nullptr;
        pipelineLayoutCreateInfo.flags                  = 0;
        pipelineLayoutCreateInfo.setLayoutCount         = 0;
        pipelineLayoutCreateInfo.pSetLayouts            = nullptr;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges    = nullptr;
    }
    
    void App::Device::populateAttachmentDescription(VkAttachmentDescription& attachmentDescription) const noexcept
    {
        attachmentDescription.flags           = 0;
        attachmentDescription.format          = this->surfaceFormat_.format;;
        attachmentDescription.samples         = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescription.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescription.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescription.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescription.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDescription.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDescription.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    
    void App::Device::populateAttachmentReference(VkAttachmentReference& attachmentReference) const noexcept
    {
        attachmentReference.attachment  = 0;
        attachmentReference.layout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    
    void App::Device::populateSubpassDescription(VkSubpassDescription& subpassDescription,
                                                 const VkAttachmentReference* pColorAttachmentReference) const noexcept
    {
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
    }
    
    void App::Device::populateRenderPassCreateInfo(VkRenderPassCreateInfo& renderPassCreateInfo,
                                                   const std::vector<VkAttachmentDescription>& attachmentDescriptions,
                                                   const std::vector<VkSubpassDescription>& subpassDescriptions) const noexcept
    {
        renderPassCreateInfo.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.pNext              = nullptr;
        renderPassCreateInfo.flags              = 0;
        renderPassCreateInfo.attachmentCount    = static_cast<std::uint32_t>(attachmentDescriptions.size());
        renderPassCreateInfo.pAttachments       = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount       = static_cast<std::uint32_t>(subpassDescriptions.size());
        renderPassCreateInfo.pSubpasses         = subpassDescriptions.data();
        renderPassCreateInfo.dependencyCount    = 0;
        renderPassCreateInfo.pDependencies      = nullptr;
    }
    
    void App::Device::populateGraphicsPipelineCreateInfo(VkGraphicsPipelineCreateInfo& graphicsPipelineCreateInfo,
                                                         const std::vector<VkPipelineShaderStageCreateInfo>& pipelineShaderStageCreateInfos,
                                                         const VkPipelineVertexInputStateCreateInfo* pPipelineVertexInputStateCreateInfo,
                                                         const VkPipelineInputAssemblyStateCreateInfo* pPipelineInputAssemblyStateCreateInfo,
                                                         const VkPipelineTessellationStateCreateInfo* pPipelineTessellationStateCreateInfo,
                                                         const VkPipelineViewportStateCreateInfo* pPipelineViewportStateCreateInfo,
                                                         const VkPipelineRasterizationStateCreateInfo* pPipelineRasterizationStateCreateInf,
                                                         const VkPipelineMultisampleStateCreateInfo* pPipelineMultisampleStateCreateInfo,
                                                         const VkPipelineDepthStencilStateCreateInfo* pPipelineDepthStencilStateCreateInfo,
                                                         const VkPipelineColorBlendStateCreateInfo* pPipelineColorBlendStateCreateInfo,
                                                         const VkPipelineDynamicStateCreateInfo* pPipelineDynamicStateCreateInfo) const noexcept
    {
        graphicsPipelineCreateInfo.sType                = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.pNext                = nullptr;
        graphicsPipelineCreateInfo.flags                = 0;
        graphicsPipelineCreateInfo.stageCount           = static_cast<std::uint32_t>(pipelineShaderStageCreateInfos.size());
        graphicsPipelineCreateInfo.pStages              = pipelineShaderStageCreateInfos.data();
        graphicsPipelineCreateInfo.pVertexInputState    = pPipelineVertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState  = pPipelineInputAssemblyStateCreateInfo;
        graphicsPipelineCreateInfo.pTessellationState   = pPipelineTessellationStateCreateInfo;
        graphicsPipelineCreateInfo.pViewportState       = pPipelineViewportStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState  = pPipelineRasterizationStateCreateInf;
        graphicsPipelineCreateInfo.pMultisampleState    = pPipelineMultisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState   = pPipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState     = pPipelineColorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState        = pPipelineDynamicStateCreateInfo;
        graphicsPipelineCreateInfo.layout               = this->pipelineLayout_;
        graphicsPipelineCreateInfo.renderPass           = this->renderPass_;
        graphicsPipelineCreateInfo.subpass              = 0;
        graphicsPipelineCreateInfo.basePipelineHandle   = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineIndex    = -1;
    }
    
    bool App::Device::checkValidationLayerSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateInstanceLayerProperties(&propertyCound, nullptr);
        
        std::vector<VkLayerProperties> properties(propertyCound);
        vkEnumerateInstanceLayerProperties(&propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->validationLayers_)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.layerName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!propertyFound)
                std::cerr << "Validation layer NOT supported: " << requiredProperty << std::endl;
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            std::cout << "All validation layers are supported!" << std::endl;
        
        return allPropertiesFound;
    }
    
    bool App::Device::checkInstanceExtensionSupport() const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, nullptr);
        
        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateInstanceExtensionProperties(nullptr, &propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->instanceExtensions_)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
            if (!propertyFound)
                std::cerr << "Instance extension NOT supported: " << requiredProperty << std::endl;
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
        if (allPropertiesFound)
            std::cout << "All instance extensions are supported!" << std::endl;
        
        return allPropertiesFound;
    }
    
    bool App::Device::checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const noexcept
    {
        std::uint32_t propertyCound = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCound, nullptr);
        
        std::vector<VkExtensionProperties> properties(propertyCound);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &propertyCound, properties.data());
        
        bool allPropertiesFound = true;
        for (const auto& requiredProperty : this->deviceExtensions_)
        {
            bool propertyFound = false;
            for (const auto& supportedProperty : properties)
                if (std::strcmp(requiredProperty, supportedProperty.extensionName) == 0)
                {
                    propertyFound = true;
                    break;
                }
#ifdef DEBUG
            if (!propertyFound)
                std::cerr << "Device extension NOT supported: " << requiredProperty << std::endl;
#endif
            allPropertiesFound = propertyFound ? allPropertiesFound : false;
        }
#ifdef DEBUG
        if (allPropertiesFound)
            std::cout << "All device extensions are supported!" << std::endl;
#endif
        return allPropertiesFound;
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
    
    void App::Device::setupSurfaceCapabilities() noexcept
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(this->physicalDevice_, this->surface_, &this->surfaceCapabilities_);
    }
    
    void App::Device::setupSurfaceFormat() noexcept
    {
        std::uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice_, this->surface_, &formatCount, nullptr);
        
        std::vector<VkSurfaceFormatKHR> surfaceFormats(static_cast<std::size_t>(formatCount));
        vkGetPhysicalDeviceSurfaceFormatsKHR(this->physicalDevice_, this->surface_, &formatCount, surfaceFormats.data());
        
        this->surfaceFormat_ = surfaceFormats[0];
        for (const auto& availableSurfaceFormat : surfaceFormats)
            if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                this->surfaceFormat_ = availableSurfaceFormat;
    }
    
    void App::Device::setupPresentMode() noexcept
    {
        std::uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice_, this->surface_, &presentModeCount, nullptr);
        
        std::vector<VkPresentModeKHR> presentModes(static_cast<std::size_t>(presentModeCount));
        vkGetPhysicalDeviceSurfacePresentModesKHR(this->physicalDevice_, this->surface_, &presentModeCount, presentModes.data());
        
        this->presentMode_ = VK_PRESENT_MODE_FIFO_KHR;
        for (const auto& availablePresentMode : presentModes)
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                this->presentMode_ = availablePresentMode;
    }
    
    void App::Device::setupExtent(const RenderWindow& window) noexcept
    {
        this->extent_ =
        {
            std::clamp(window.GetFramebufferSize().width, this->surfaceCapabilities_.minImageExtent.width, this->surfaceCapabilities_.maxImageExtent.width),
            std::clamp(window.GetFramebufferSize().height, this->surfaceCapabilities_.minImageExtent.height, this->surfaceCapabilities_.maxImageExtent.height)
        };
    }
    
    std::vector<char> App::Device::readFile(const std::string& filePath) const
    {
        std::ifstream fileStream(filePath, std::ios::ate | std::ios::binary);
        
        if (!fileStream.is_open())
            throw std::runtime_error("Failed to open file: " + filePath);
        
        std::size_t fileSize = static_cast<std::size_t>(fileStream.tellg());
        std::vector<char> buffer(fileSize);
        
        fileStream.seekg(0);
        fileStream.read(buffer.data(), fileSize);
        fileStream.close();
        
        return buffer;
    }
            

#pragma mark - App
    App::App()
    :
    appName_("Mangos"),
    window_(this->appName_, 500, 500),
    device_(this->appName_, this->window_)
    {}
    
    App::~App()
    {}
    
    void App::run()
    {
        while (!this->window_.shouldClose())
            this->window_.pollEvents();
    }
}

