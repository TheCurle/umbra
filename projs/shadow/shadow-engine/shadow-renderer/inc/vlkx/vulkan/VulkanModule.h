#pragma once

#include <vlkx/vulkan/ValidationAndExtension.h>
#include <vlkx/vulkan/VulkanDevice.h>
#include <vlkx/vulkan/SwapChain.h>

#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <vlkx/vulkan/Tools.h>

#include <SDL_vulkan.h>
#include "core/Module.h"

class VulkanModule : public ShadowEngine::Module {
    SHObject_Base(VulkanModule);
public:

    VulkanModule();
	~VulkanModule() override;

#ifdef _DEBUG
	static const bool validationRequired = true;
#else
	static const bool validationRequired = false;
#endif

    void PreInit() override;

    void Init() override;

    void Update() override;

    void PreRender() override;

    void Render() override;

    void LateRender() override;

    void AfterFrameEnd() override;

    void Destroy() override;

    void Event(SDL_Event* e) override;

	// VulkanModule is a singleton class.
	static VulkanModule* instance;
	static VulkanModule* getInstance();

	// Initialize all Vulkan context and prepare validations in debug mode.
	void initVulkan(SDL_Window* window);
	void createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension* validations);

	// Start and end a frame render.
	void startDraw();
	void endDraw();

	// Cleanup after the application has closed.
	void cleanup();

	VkInstance getVulkan() { return vulkan; }
	VulkanDevice* getDevice() { return device; }
	SwapChain* getSwapchain() { return swapchain; }
	VmaAllocator getAllocator() { return allocator; }

    int getMaxFrames() { return MAX_FRAMES; }

private:
    // To keep track of the window during... stuff
    SDL_Window* wnd;

	// To handle the validation of Vulkan API usage (because fuck you, it's your problem now)
	ValidationAndExtension* validators{};
	// To manage interaction with the hardware
	VulkanDevice* device{};
	// To handle the framebuffers
	SwapChain* swapchain{};

	// To handle automatic management of memory.
	VmaAllocator allocator{};

	// To manage the Vulkan context that was passed to us by the API
	VkInstance vulkan{};
	// To manage the canvas that was given to us by GLFW
	VkSurfaceKHR surface{};

	// The maximum number of frames that can be dealt with at a time.
	const int MAX_FRAMES = 2; // Double-buffering requires two frames in memory

};