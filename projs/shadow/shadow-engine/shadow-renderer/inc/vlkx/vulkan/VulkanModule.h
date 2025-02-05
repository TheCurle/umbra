#pragma once

#include <vlkx/vulkan/ValidationAndExtension.h>
#include <vlkx/vulkan/VulkanDevice.h>

#include <vulkan/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <SDL_vulkan.h>
#include <core/Module.h>
#include "SwapChain.h"

namespace vlkx { class ScreenRenderPassManager; }

class VulkanModule : public ShadowEngine::RendererModule {
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

    void Recreate() override;

    void Update(int frame) override;

    void PreRender() override;

    void Render(VkCommandBuffer& commands, int frame) override;

    void OverlayRender() override;

    void LateRender(VkCommandBuffer& commands, int frame) override;

    void AfterFrameEnd() override;

    void Destroy() override;

    void Event(SDL_Event* e) override;

    void BeginRenderPass(const std::unique_ptr<vlkx::RenderCommand>& commands) override;

    void EnableEditor() override;

    VkExtent2D GetRenderExtent() override;

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
    SDL_Window* getWind() { return wnd; }
    const std::unique_ptr<vlkx::ScreenRenderPassManager>& getRenderPass();


private:
    bool editorEnabled = false;
    std::vector<VkDescriptorSet> editorRenderPlanes;
    std::vector<std::unique_ptr<vlkx::Image>> editorContentFrames;

    // The SDL Window contains the size of the drawable area.
    SDL_Window* wnd;
	// To handle the validation of Vulkan API usage
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

};