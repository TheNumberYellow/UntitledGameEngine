#include "EnginePlatform.h"

#include "RendererPlatform.h"

#include "vulkan/vulkan.h"

namespace 
{
    VkInstance Instance;
}

Renderer::Renderer()
{

    VkApplicationInfo AppInfo{};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Game Engine";
    AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.pEngineName = "Game Engine";
    AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &AppInfo;

    
}

Renderer::~Renderer()
{

    vkDestroyInstance(Instance, nullptr);
}