#define VMA_IMPLEMENTATION

#include <vulkan/vk_mem_alloc.h>

#define VKTOOLS_IMPLEMENTATION

#include <vlkx/vulkan/VulkanManager.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

VulkanManager::VulkanManager() { }

VulkanManager::~VulkanManager() = default;

VulkanManager* VulkanManager::instance = nullptr;


VmaAllocator             VkTools::g_allocator;
VkDevice                 VkTools::g_Device = VK_NULL_HANDLE;

VulkanManager* VulkanManager::getInstance() {
    return VulkanManager::instance != nullptr ? VulkanManager::instance
                                              : (VulkanManager::instance = new VulkanManager());
}

void VulkanManager::createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension* validations) {
    VkApplicationInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Sup";
    info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.pEngineName = "Infinity Drive";
    info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;


    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    #ifdef __APPLE__
        VkFlags instanceFlag = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    #else
        VkFlags instanceFlag = 0;
    #endif

    instanceInfo.flags = instanceFlag;
    instanceInfo.pApplicationInfo = &info;

    auto extensions = validations->getRequiredExtensions(wnd, true);
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.ppEnabledExtensionNames = &extensions[0];

    auto layers = validations->requiredValidations;
    if (enableValidation) {
        instanceInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        instanceInfo.ppEnabledLayerNames = &layers[0];
    } else {
        instanceInfo.enabledLayerCount = 0;
    }

    auto status = vkCreateInstance(&instanceInfo, nullptr, &vulkan);
    if (status != VK_SUCCESS) {
        throw std::runtime_error("Failed to initialize Vulkan: " + std::to_string(status));
    }

}

void VulkanManager::initVulkan(SDL_Window* window) {
    wnd = window;
    validators = new ValidationAndExtension();

    spdlog::info("Initializing Infinity Drive rendering engine");
    spdlog::default_logger()->set_level(spdlog::level::debug);

    if (!validators->checkValidationSupport())
        throw std::runtime_error("Validation not available");

    createAppAndVulkanInstance(validationRequired, validators);

    validators->setupDebugCallback(validationRequired, vulkan);

    if (SDL_Vulkan_CreateSurface(window, vulkan, &surface) != SDL_TRUE)
        throw std::runtime_error("Unable to create Vulkan Surface");

    this->device = new VulkanDevice();
    this->device->choosePhysicalDevice(&vulkan, surface);
    this->device->createLogicalDevice(surface, validationRequired, validators);

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = this->device->physical;
    allocatorInfo.device = this->device->logical;
    allocatorInfo.instance = this->vulkan;
    vmaCreateAllocator(&allocatorInfo, &this->allocator);
    VkTools::g_allocator = this->allocator;
    VkTools::g_Device = this->device->logical;

    this->swapchain = new SwapChain();
    this->swapchain->create(surface);

    spdlog::info("Infinity Drive initialization finished.");
}


void VulkanManager::cleanup() {
    // Wait for the GPU to not be busy
    vkDeviceWaitIdle(VulkanManager::getInstance()->getDevice()->logical);

    swapchain->destroy();

    // Destroy the Vulkan Device
    VulkanManager::getInstance()->getDevice()->destroy();

    // Delete the layer validators.
    validators->destroy(validationRequired, vulkan);

    // Delete the surface and Vulkan instance.
    vkDestroySurfaceKHR(vulkan, surface, nullptr);
    vkDestroyInstance(vulkan, nullptr);

    vmaDestroyAllocator(allocator);

    // Delete allocated memory for our own data.
    delete swapchain;
    delete device;
    delete validators;
}