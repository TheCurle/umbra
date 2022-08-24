#include "ShadowApplication.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Time.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl.h"
#include <vlkx/vulkan/VulkanManager.h>
#include <vlkx/render/Camera.h>
#include <vlkx/render/render_pass/ScreenRenderPass.h>
#include "spdlog/spdlog.h"
#include "vlkx/vulkan/abstraction/Commands.h"
#include "vlkx/render/Geometry.h"
#include "vlkx/render/shader/Pipeline.h"
#include "temp/model/Builder.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    struct Transform {
        alignas(sizeof(glm::mat4)) glm::mat4 value;
    };

    std::unique_ptr<vlkx::ScreenRenderPassManager> passManager;
    std::unique_ptr<vlkx::RenderCommand> renderCommands;

    std::unique_ptr<vlkx::PushConstant> transformation;
    std::unique_ptr<vlkxtemp::Model> model;

    float aspectRatio;

	ShadowApplication* ShadowApplication::instance = nullptr;
	
	ShadowApplication::ShadowApplication(int argc, char* argv[])
	{
		instance = this;

		if(argc > 1)
		{
			for (size_t i = 0; i < argc; i++)
			{
				std::string param(argv[i]);
				if(param == "-no-gui")
				{
					this->no_gui = true;
				}
                if(param == "-game")
                {
                    this->game = argv[i+1];
                }
			}
		}

		//game = _setupFunc();
	}


	ShadowApplication::~ShadowApplication()
	{
	}

	void ShadowApplication::Init()
	{
        // Initialize SDL. SDL_Init will return -1 if it fails.
        if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
            //std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
            //system("pause");
            // End the program
            //return 1;
        }

        window_ = new ShadowWindow(800,450);

        CATCH(VulkanManager::getInstance()->initVulkan(window_->sdlWindowPtr);)

        renderCommands = std::make_unique<vlkx::RenderCommand>(2);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        VkDescriptorPool imGuiPool;
        VulkanManager* vk = VulkanManager::getInstance();
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
        vkCreateDescriptorPool(vk->getDevice()->logical, &pool_info, VK_NULL_HANDLE, &imGuiPool);

        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForVulkan(window_->sdlWindowPtr);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = vk->getVulkan();
        init_info.PhysicalDevice = vk->getDevice()->physical;
        init_info.Device = vk->getDevice()->logical;
        init_info.QueueFamily = vk->getDevice()->queueData.graphics;
        init_info.Queue = vk->getDevice()->graphicsQueue;
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = imGuiPool;
        init_info.Subpass = 1;
        init_info.MinImageCount = vk->getSwapchain()->images.size();
        init_info.ImageCount = vk->getSwapchain()->images.size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = nullptr;

        vlkxtemp::ModelBuilder::ShaderPool pool;

        renderCommands = std::make_unique<vlkx::RenderCommand>(2);
        transformation = std::make_unique<vlkx::PushConstant>(sizeof(Transform), 2);

        using vlkxtemp::ModelBuilder;

        aspectRatio = (float) window_->Width / window_->Height;

        model = ModelBuilder { "Walrus", 2, aspectRatio, ModelBuilder::SingleMeshModel {
            "resources/walrus/walrus.obj", 1, {{ ModelBuilder::TextureType::Diffuse, { { "resources/walrus/texture.png" } } } }
            }}
            .bindTextures(ModelBuilder::TextureType::Diffuse, 1)
            .pushStage(VK_SHADER_STAGE_VERTEX_BIT)
            .pushConstant(transformation.get(), 0)
            .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/walrus/cube.vert.spv")
            .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/walrus/cube.frag.spv")
            .build();

        passManager = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 2 });

        passManager->initializeRenderPass();

        model->update(true, VulkanManager::getInstance()->getSwapchain()->extent, VK_SAMPLE_COUNT_1_BIT, *passManager->getPass(), 0);

        ImGui_ImplVulkan_Init(&init_info, **passManager->getPass());
        // Upload Fonts
        VkTools::immediateExecute([](const VkCommandBuffer& commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); }, VulkanManager::getInstance()->getDevice());

    }

    void updateData(int frame) {
        const float elapsed_time = Time::timeSinceStart;
        const auto window = VulkanManager::getInstance()->getWind();
        const glm::mat4 model = glm::rotate(glm::mat4{1.0f},
                                            elapsed_time * glm::radians(90.0f),
                                            glm::vec3{1.0f, 1.0f, 0.0f});
        const glm::mat4 view = glm::lookAt(glm::vec3{3.0f}, glm::vec3{0.0f},
                                           glm::vec3{0.0f, 0.0f, 1.0f});
        const glm::mat4 proj = glm::perspective(
                glm::radians(45.0f), aspectRatio,
                0.1f, 100.0f);
        *transformation->getData<Transform>(frame) = {proj * view * model};
    }

    void imGuiStartDraw() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }
    void imGuiEndDraw(const VkCommandBuffer& commands) {
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commands);
    }

    void ShadowApplication::PollEvents() {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {  // poll until all events are handled!
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                running = false;
        }
    }

	void ShadowApplication::Start()
	{
        const auto update = [](const int frame) { updateData(frame); };

		while (running)
		{
            PollEvents();

            const auto result = renderCommands->execute(renderCommands->getFrame(), VulkanManager::getInstance()->getSwapchain()->swapChain, update,
            [](const VkCommandBuffer& buffer, uint32_t frame) {
                passManager->getPass()->execute(buffer, frame, {
                        // Render our model
                        [&frame](const VkCommandBuffer& commands) {
                            model->draw(commands, frame, 1);
                        },
                        // Render ImGUI
                        [&frame](const VkCommandBuffer& commands) {
                            imGuiStartDraw();

                            bool showDemo = true;
                            if (showDemo)
                                ImGui::ShowDemoWindow(&showDemo);

                            imGuiEndDraw(commands);
                        }
                });
            });

            if (result.has_value())
                throw std::runtime_error("Resizing is not implemented");

            renderCommands->nextFrame();

            Time::UpdateTime();
		}

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyWindow(window_->sdlWindowPtr);
        SDL_Quit();
	}
}
