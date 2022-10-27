#include "App.hpp"

namespace mgo
{
#pragma mark - App
    
    void App::Run()
    {
        this->Init();
        this->MainLoop();
        this->Destroy();
    }
    
    void App::Init()
    {
        Window window(800, 600, "Mangos Eninge");
        VulkanInstance vulkan("Mangos Eninge", "Mangos");
        
        
    }
    
    void App::MainLoop()
    {
        
    }
        
    void App::Destroy()
    {
    }
        
#pragma mark - App::Window
    
    App::Window::Window(int width, int hight, const std::string& name)
    :
    width(width), hight(hight), name(name)
    {
        glfwSetErrorCallback(ErrorCallback);
        
        if (!glfwInit())
            throw std::runtime_error("Failed to init GLFW.");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        this->window = glfwCreateWindow(this->width, this->hight, name.c_str(), nullptr, nullptr);
        
        if (!this->window)
            throw std::runtime_error("Failed to create GLFW window.");
    }
    
    App::Window::~Window() noexcept
    {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }
    
    bool App::Window::ShouldClose() noexcept
    {return glfwWindowShouldClose(this->window);}

    
    void App::Window::ErrorCallback(int error, const char* description)
    {throw std::runtime_error(std::to_string(error) + " : " + std::string(description));}

#pragma mark - App::VulkanIntance
    
    App::VulkanInstance::VulkanInstance(const std::string& app_name, const std::string& engine_name)
    {
        ValidationLayers validation_layers;
        
        std::uint32_t glfw_extension_count = 0;
        std::vector<const char*> required_extensions;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        for(std::uint32_t i = 0; i < glfw_extension_count; i++)
            required_extensions.emplace_back(glfw_extensions[i]);

        required_extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        
        VkApplicationInfo app_info;
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = app_name.c_str();
        app_info.pEngineName = engine_name.c_str();
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;
        
        VkInstanceCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = glfw_extension_count;
        create_info.ppEnabledExtensionNames = glfw_extensions;
        create_info.enabledLayerCount = 0;
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        create_info.enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size());
        create_info.ppEnabledExtensionNames = required_extensions.data();

        if (validation_layers.enable)
        {
            create_info.enabledLayerCount = static_cast<std::uint32_t>(validation_layers.validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }
        
        if (vkCreateInstance(&create_info, nullptr, &this->vulkan_instance) != VK_SUCCESS)
            throw std::runtime_error("Failed to create instance.");
    }
    
    App::VulkanInstance::~VulkanInstance()
    {vkDestroyInstance(this->vulkan_instance, nullptr);}
    
#pragma mark - App::ValidationLayers
    
    App::ValidationLayers::ValidationLayers()
#ifdef NDEBUG
    : enable(false)
#else
    : enable(true)
#endif
    {
        if (enable && !CheckSupport())
            throw std::runtime_error("Validation layers requested, but not available.");
            
    }
    
    App::ValidationLayers::~ValidationLayers()
    {
        
    }
    
    bool App::ValidationLayers::CheckSupport() noexcept
    {
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char* layer_name : this->validation_layers)
        {
            bool layer_found = false;
            for (const auto& layer_properties : available_layers)
            {
                if (!strcmp(layer_name, layer_properties.layerName))
                {
                    layer_found = true;
                    break;
                }
            }
            if (!layer_found)
                return false;
        }
        return true;
    }

#pragma mark - App::Pipeline

    App::Pipeline::Pipeline(const std::string& frag_name, const std::string& vert_name)
    {
        
    }
    
    std::vector<char> App::Pipeline::ReadFile(const std::string& file_name)
    {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);
        
        if (!file.is_open())
            throw std::runtime_error("Failed to open file: " + file_name);
        
        std::size_t file_size = static_cast<std::size_t>(file.tellg());

        std::vector<char> buffer(file_size);
        
        file.seekg(0);
        file.read(buffer.data(), file_size);
        
        return buffer;
    }
}
