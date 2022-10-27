#ifndef App_hpp
#define App_hpp
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define MGO_USE_MATRIX_TYPEDEF
#define MGO_USE_MATHS_DEFINES
#include "MangosMaths.hpp"

namespace mgo
{
    class App final
    {
    public:
        void Run();
        
    private:
        void Init();
        
        void MainLoop();
        
        void Destroy();
        
        class Window final
        {
            friend App;
            
            const int width, hight;
            
            const std::string name;
            
            GLFWwindow* window;
            
            Window(int width, int hight, const std::string& name);
            
            ~Window() noexcept;
            
            bool ShouldClose() noexcept;
            
            static void ErrorCallback(int error, const char* description);
        };
        
        class VulkanInstance final
        {
            friend App;
            
            VkInstance vulkan_instance;
            
            VulkanInstance(const std::string& app_name, const std::string& engine_name);
            
            ~VulkanInstance();
        };
        
        class ValidationLayers final
        {
            friend App;
            
            bool enable;
            
            const std::vector<const char*> validation_layers;
            
            ValidationLayers();
            
            ~ValidationLayers();
            
            bool CheckSupport() noexcept;
        };
        
        class Pipeline final
        {
            friend App;
            
            Pipeline(const std::string& frag_name, const std::string& vert_name);
            
            std::vector<char> ReadFile(const std::string& file_name);
        };
        
    };
}
#endif
