#define VMA_IMPLEMENTATION

#include <vulkan/vk_mem_alloc.h>
#include <vlkx/vulkan/Tools.h>

#include <vlkx\vulkan\VulkanModule.h>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"
#include "vlkx/render/render_pass/ScreenRenderPass.h"
#include <vlkx/vulkan/SwapChain.h>

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(VulkanModule)

std::unique_ptr<vlkx::ScreenRenderPassManager> renderPass;

VulkanModule::VulkanModule() { instance = this; }

VulkanModule::~VulkanModule() = default;

VulkanModule* VulkanModule::instance = nullptr;

VulkanModule* VulkanModule::getInstance() {
    return VulkanModule::instance != nullptr ? VulkanModule::instance
                                             : (VulkanModule::instance = new VulkanModule());
}

void VulkanModule::PreInit() {
    spdlog::info("Vulkan Renderer Module loading..");


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
    ImGui_ImplSDL2_InitForVulkan(wnd);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = getVulkan();
    init_info.PhysicalDevice = getDevice()->physical;
    init_info.Device = getDevice()->logical;
    init_info.QueueFamily = getDevice()->queueData.graphics;
    init_info.Queue = getDevice()->graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = imGuiPool;
    init_info.Subpass = 1;
    init_info.MinImageCount = getSwapchain()->images.size();
    init_info.ImageCount = getSwapchain()->images.size();
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.CheckVkResultFn = nullptr;

    renderPass = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 2 } );
    renderPass->initializeRenderPass();

    ImGui_ImplVulkan_Init(&init_info, **renderPass->getPass());

    VkTools::immediateExecute([](const VkCommandBuffer& commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); }, getDevice());

}

void VulkanModule::Init() {
}

void VulkanModule::BeginRenderPass(const std::unique_ptr<vlkx::RenderCommand>& commands) {
    const auto result = commands->execute(commands->getFrame(), swapchain->swapChain, [](const int frame) { ShadowEngine::ModuleManager::instance->Update(frame); },
            [this](const VkCommandBuffer& buffer, int frame) {
                renderPass->getPass()->execute(buffer, frame, {
                    // Render our model
                    [&frame](const VkCommandBuffer& commands) {
                        ShadowEngine::ModuleManager::instance->Render(const_cast<VkCommandBuffer &>(commands), frame);
                        ShadowEngine::ModuleManager::instance->LateRender(const_cast<VkCommandBuffer &>(commands), frame);
                    },
                    // Render ImGUI
                    [&](const VkCommandBuffer& commands) {
                        ImGui_ImplVulkan_NewFrame();
                        ImGui_ImplSDL2_NewFrame();
                        ImGui::NewFrame();

                        ShadowEngine::ModuleManager::instance->OverlayRender();

                        ImGui::Render();
                        ImGuiIO& io = ImGui::GetIO(); (void)io;

                        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands);

                        // Update and Render additional Platform Windows
                        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                            ImGui::UpdatePlatformWindows();
                            ImGui::RenderPlatformWindowsDefault();
                        }
                    }
                });
            }
    );
}

void VulkanModule::PreRender() {}
void VulkanModule::OverlayRender() {}
void VulkanModule::AfterFrameEnd() {}
void VulkanModule::Render(VkCommandBuffer& commands, int frame) {}
void VulkanModule::Update(int frame) {}
void VulkanModule::Event(SDL_Event *e) { (void)e; }

void VulkanModule::LateRender(VkCommandBuffer& commands, int frame) {
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
    VkTools::allocator = allocator;

    this->swapchain = new SwapChain();
    this->swapchain->create(surface);
    spdlog::info("Infinity Drive initialization finished.");
}

void VulkanModule::cleanup() {
    // Wait for the GPU to not be busy
    vkDeviceWaitIdle(VulkanModule::getInstance()->getDevice()->logical);

    swapchain->destroy();

    // Destroy the Vulkan Device
    VulkanModule::getInstance()->getDevice()->destroy();

    // Delete the layer validators.
    validators->destroy(validationRequired, vulkan);

    // Delete the surface and Vulkan instance.
    vkDestroySurfaceKHR(vulkan, surface, nullptr);
    vkDestroyInstance(vulkan, nullptr);

    vmaDestroyAllocator(allocator);

    delete swapchain;
    delete device;
    delete validators;
}