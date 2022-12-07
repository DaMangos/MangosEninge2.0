#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <exception>
namespace mgo
{
    namespace glfw
    {
#pragma mark - mgo::glfw::Window
        class Window final
        {
        private:
            GLFWwindow* pWindow_;
            const std::string windowName_;
            const std::uint32_t windowHeight_;
            const std::uint32_t windowWidth_;
            bool framebufferResized_;
            
        public:
            Window(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHeight);
            
            ~Window() noexcept;
            
            const GLFWwindow* Get() const noexcept;
            
            std::vector<const char*> getExtensions() const noexcept;
            
            VkResult createSurface(VkInstance instance, VkSurfaceKHR* pSurface) const;
            
            VkExtent2D GetFramebufferSize() const noexcept;
            
            bool shouldClose() const noexcept;
            
            void pollEvents() noexcept;
            
            bool hasResized() noexcept;
            
        private:
            static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
            
            static void errorCallback(int error, const char* description) noexcept;
        };
    }
}
