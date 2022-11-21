#define VMA_IMPLEMENTATION

#include <vulkan/vk_mem_alloc.h>
#include <vlkx/vulkan/Tools.h>

#include <vlkx\vulkan\VulkanModule.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(VulkanModule)

VulkanModule::VulkanModule() { instance = this; }

VulkanModule::~VulkanModule() = default;

VulkanModule* VulkanModule::instance = nullptr;

VulkanModule* VulkanModule::getInstance() {
    return VulkanModule::instance != nullptr ? VulkanModule::instance
                                             : (VulkanModule::instance = new VulkanModule());
}

void VulkanModule::PreInit() {
    spdlog::info("Vulkan Renderer Module loading..");
}

void VulkanModule::Init() {

    auto shApp = ShadowEngine::ShadowApplication::Get();

    ShadowEngine::ModuleManager &moduleManager = shApp.GetModuleManager();

    auto sdl2module = moduleManager.GetModuleByType<ShadowEngine::SDL2Module>();

    CATCH(initVulkan(sdl2module->_window->sdlWindowPtr);)

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    VkDescriptorPool imGuiPool;
    VkDescriptorPoolSize pool_sizes[] =
            {
                    { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(getDevice()->logical, &pool_info, VK_NULL_HANDLE, &imGuiPool);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForVulkan(sdl2module->_window->sdlWindowPtr);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = getVulkan();
    init_info.PhysicalDevice = getDevice()->physical;
    init_info.Device = getDevice()->logical;
    init_info.QueueFamily = getDevice()->queueData.graphics;
    init_info.Queue = getDevice()->graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imGuiPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = getSwapchain()->images.size();
    init_info.ImageCount = getSwapchain()->images.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&init_info, getRenderPass()->pass);

    // Upload Fonts
    {
        // Prepare to create a temporary command pool.
        VkCommandPool pool;
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = getDevice()->queueData.graphics;
        poolCreateInfo.flags = 0;

        // Create the pool
        if (vkCreateCommandPool(getDevice()->logical, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate a temporary command pool");

        VkCommandBuffer buffer = VkTools::createTempCommandBuffer(pool, getDevice()->logical);

        ImGui_ImplVulkan_CreateFontsTexture(buffer);

        VkSubmitInfo end_info = {};
        end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        end_info.commandBufferCount = 1;
        end_info.pCommandBuffers = &buffer;

        VkTools::executeAndDeleteTempBuffer(buffer, pool, getDevice()->graphicsQueue, getDevice()->logical);

        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

}

void VulkanModule::PreRender() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    startDraw();
}

void VulkanModule::AfterFrameEnd() {
    VulkanModule::getInstance()->endDraw();
}

void VulkanModule::Render() {}
void VulkanModule::Update() {}

void VulkanModule::Event(SDL_Event *e) { (void)e; }

void VulkanModule::LateRender() {

    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VulkanModule::getInstance()->getCurrentCommandBuffer());

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void VulkanModule::Destroy() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void VulkanModule::createAppAndVulkanInstance(bool enableValidation, ValidationAndExtension* validations) {
    VkApplicationInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    info.pApplicationName = "Sup";
    info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.pEngineName = "Infinity Drive";
    info.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instanceInfo = {};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
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

void VulkanModule::initVulkan(SDL_Window* window) {
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

    this->swapchain = new SwapChain();
    this->swapchain->create(surface);

    this->renderPass = new RenderPass();

    // Set up for vertex rendering
    this->renderPass->createVertexRenderPass(swapchain->format);
    this->renderTexture = new SingleRenderTexture();

    this->renderTexture->createViewsAndFramebuffer(swapchain->images, swapchain->format,
        swapchain->extent,renderPass->pass
    );

    this->buffers = new CommandBuffer();
    this->buffers->createCommandPoolAndBuffers(swapchain->images.size());

    // Create semaphores for render events
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &newImageSem);
    vkCreateSemaphore(device->logical, &semaphoreInfo, nullptr, &renderDoneSem);

    // Create fences for the frames
    inFlight.resize(MAX_FRAMES);
    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES; i++) {
        if (vkCreateFence(device->logical, &fenceInfo, nullptr, &inFlight[i]) != VK_SUCCESS)
            throw std::runtime_error("Unable to create fence for a frame");
    }

    spdlog::info("Infinity Drive initialization finished.");
}

void VulkanModule::startDraw() {
    // Prepare for a new frame to start
    vkAcquireNextImageKHR(device->logical, swapchain->swapChain,
        std::numeric_limits<uint64_t>::max(), newImageSem,VK_NULL_HANDLE, &imageIndex
    );

    vkWaitForFences(device->logical, 1, &inFlight[imageIndex], VK_TRUE,
        std::numeric_limits<uint64_t>::max()
    );

    vkResetFences(device->logical, 1, &inFlight[imageIndex]);

    // Fetch the next command buffer
    currentCommandBuffer = buffers->buffers[imageIndex];
    buffers->beginCommandBuffer(currentCommandBuffer);

    // Setup render pass; setup clear color
    VkClearValue clearColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red

    // Execute render pass
    renderPass->beginRenderPass({ clearColor }, currentCommandBuffer, dynamic_cast<SingleRenderTexture*>(renderTexture)->swapChainFramebuffers[imageIndex], dynamic_cast<SingleRenderTexture*>(renderTexture)->swapChainImageExtent);
}

void VulkanModule::endDraw() {
    // End command buffer first
    renderPass->endRenderPass(currentCommandBuffer);
    buffers->endCommandBuffer(currentCommandBuffer);

    // Prepare to submit all draw commands to the GPU
    VkPipelineStageFlags waitStages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &currentCommandBuffer;
    submitInfo.pWaitDstStageMask = waitStages;
    // Wait for the New Image semaphore, and signal the Render Done semaphore when finished
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &newImageSem;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderDoneSem;

    // Submit.
    vkQueueSubmit(VulkanModule::getInstance()->getDevice()->graphicsQueue, 1, &submitInfo, inFlight[imageIndex]);

    // Prepare to show the drawn frame on the screen.
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain->swapChain;
    presentInfo.pImageIndices = &imageIndex;
    // Wait until render is finished before presenting.
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderDoneSem;

    // Show.
    vkQueuePresentKHR(VulkanModule::getInstance()->getDevice()->presentationQueue, &presentInfo);

    // Wait for the GPU to catch up
    vkQueueWaitIdle(VulkanModule::getInstance()->getDevice()->presentationQueue);
}

void VulkanModule::cleanup() {
    // Wait for the GPU to not be busy
    vkDeviceWaitIdle(VulkanModule::getInstance()->getDevice()->logical);

    // Destroy our own data
    vkDestroySemaphore(device->logical, renderDoneSem, nullptr);
    vkDestroySemaphore(device->logical, newImageSem, nullptr);

    for (size_t i = 0; i < MAX_FRAMES; i++) {
        vkDestroyFence(device->logical, inFlight[i], nullptr);
    }

    buffers->destroy();
    renderTexture->destroy();
    renderPass->destroy();
    swapchain->destroy();

    // Destroy the Vulkan Device
    VulkanModule::getInstance()->getDevice()->destroy();

    // Delete the layer validators.
    validators->destroy(validationRequired, vulkan);

    // Delete the surface and Vulkan instance.
    vkDestroySurfaceKHR(vulkan, surface, nullptr);
    vkDestroyInstance(vulkan, nullptr);

    vmaDestroyAllocator(allocator);

    // Delete allocated memory for our own data.
    delete buffers;
    delete renderTexture;
    delete renderPass;
    delete swapchain;
    delete device;
    delete validators;
}