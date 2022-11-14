#include "glfw.hpp"

#pragma mark - LOG(message)
#ifndef LOG
#ifdef DEBUG
#define LOG(message) std::cerr << message << std::endl
#else
#define LOG(message)
#endif
#endif
namespace glfw
{
#pragma mark - glfw::Window
    Window::Window(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHight)
    :
    windowName_(windowName),
    windowHight_(windowHight),
    windowWidth_(windowWidth),
    pWindow_(nullptr)
    {
        glfwSetErrorCallback(this->errorCallback);
        
        if (!glfwInit())
            throw std::runtime_error("Failed to initialise GLFW!");
        
        LOG("Successfully initialise GLFW!");
        
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        
        this->pWindow_ = glfwCreateWindow(this->windowWidth_, this->windowHight_, this->windowName_.c_str(), nullptr, nullptr);
        
        if (!this->pWindow_)
            throw std::runtime_error("Failed to create GLFWwindow!");
        
        LOG("Successfully create a GLFW window!");
    }
    
    Window::~Window() noexcept
    {
        glfwDestroyWindow(this->pWindow_);
        glfwTerminate();
    }
    
    const GLFWwindow* Window::Get() const noexcept
    {
        return this->pWindow_;
    }
    
    std::vector<const char*> Window::getExtensions() const noexcept
    {
        std::uint32_t extensionCount = 0;
        const char** requiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);
        
        return std::vector<const char*>(requiredInstanceExtensions, requiredInstanceExtensions + extensionCount);
    }
    
    VkResult Window::createSurface(VkInstance instance, VkSurfaceKHR* pSurface) const
    {
        return glfwCreateWindowSurface(instance, this->pWindow_, nullptr, pSurface);
    }
    
    VkExtent2D Window::GetFramebufferSize() const noexcept
    {
        int width, height;
        
        glfwGetFramebufferSize(this->pWindow_, &width, &height);
        
        return {static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)};
    }
    
    bool Window::shouldClose() const noexcept
    {
        return glfwWindowShouldClose(this->pWindow_);
    }
    
    void Window::pollEvents() noexcept
    {
        glfwPollEvents();
    }
    
    void Window::errorCallback(int error, const char* description)
    {
        LOG("GLFW error: " << description);
    }
}
