#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#pragma mark - MGO_LOG(message)
#ifndef MGO_LOG
#ifdef DEBUG
#define MGO_LOG(message) std::cerr << message << std::endl
#else
#define MGO_LOG(message)
#endif
#endif
namespace mgo
{
    namespace glfw
    {
#pragma mark - mgo::glfw::Window
        class Window final
        {
        private:
            const std::string       windowName_;
            const std::uint32_t     windowHight_;
            const std::uint32_t     windowWidth_;
            GLFWwindow*             pWindow_;
            
        public:
            Window(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight);
            
            ~Window() noexcept;
            
            GLFWwindow* Get() const noexcept;
            
            std::vector<const char*> getExtensions() const noexcept;
            
            VkResult createSurface(VkInstance instance, VkSurfaceKHR* pSurface) const;
            
            VkExtent2D GetFramebufferSize() const noexcept;
            
            bool shouldClose() const noexcept;
            
            static void pollEvents() noexcept;
            
            static void errorCallback(int error, const char* description);
        };
    }
}
