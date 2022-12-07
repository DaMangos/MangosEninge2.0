#include "mgo_glfw.hpp"
namespace mgo
{
    namespace glfw
    {
#pragma mark - mgo::glfw::Window
        Window::Window(const std::string& windowName, std::uint32_t windowWidth, std::uint32_t windowHeight)
        :
        windowName_(windowName),
        windowHeight_(windowHeight),
        windowWidth_(windowWidth),
        framebufferResized_(false)
        {
            glfwSetErrorCallback(this->errorCallback);
            
            if (!glfwInit())
                throw std::runtime_error("Failed to initialise GLFW!");
                        
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            
            this->pWindow_ = glfwCreateWindow(this->windowWidth_, this->windowHeight_, this->windowName_.c_str(), nullptr, nullptr);
            
            if (!this->pWindow_)
                throw std::runtime_error("Failed to create mgo::glfw::Window!");
            
            glfwSetWindowUserPointer(this->pWindow_, this);
            glfwSetFramebufferSizeCallback(this->pWindow_, this->framebufferResizeCallback);
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
        
        bool Window::hasResized() noexcept
        {
            if (this->framebufferResized_)
            {
                this->framebufferResized_ = false;
                return true;
            }
            return false;
        }

        void Window::errorCallback(int error, const char* description) noexcept
        {
            MGO_DEBUG_LOG_ERROR("GLFW error: " << description);
        }
        
        void Window::framebufferResizeCallback(GLFWwindow* pWindow, int width, int height)
        {
            auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(pWindow));
            window->framebufferResized_ = true;
        }
    }
}
