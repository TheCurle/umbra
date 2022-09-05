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

    struct SkyboxTransform {
        alignas(sizeof(glm::mat4)) glm::mat4 value;
    };

    std::unique_ptr<vlkx::ScreenRenderPassManager> passManager;
    std::unique_ptr<vlkx::RenderCommand> renderCommands;

    std::unique_ptr<vlkx::UserPerspectiveCamera> camera;

    std::unique_ptr<vlkx::PushConstant> skyboxConstant;
    std::unique_ptr<vlkxtemp::Model> skyboxModel;
    std::unique_ptr<vlkxtemp::Model> planetModel;
    std::unique_ptr<vlkxtemp::Model> asteroidModel;

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
        skyboxConstant = std::make_unique<vlkx::PushConstant>(sizeof(SkyboxTransform), 2);

        vlkx::Camera::Config conf {};
        conf.pos = glm::vec3(0, 0, 0);
        conf.target = glm::vec3(1, 0, 0);
        camera = vlkx::UserPerspectiveCamera::create( {}, conf, { 45, aspectRatio });

        using vlkxtemp::ModelBuilder;

        aspectRatio = (float) window_->Width / window_->Height;

        const vlkx::RefCountedTexture::CubemapLocation skybox {
                "resources/planets/bg",
                {
                        "left.png", "right.png",
                        "top.png", "bottom.png",
                        "front.png", "back.png"
                }
        };

        skyboxModel = ModelBuilder {
            "Skybox", 2, aspectRatio,
            ModelBuilder::SingleMeshModel { "resources/planets/skybox.obj", 1,
                { { ModelBuilder::TextureType::Cubemap, { { skybox } } } }
            }}
            .bindTextures(ModelBuilder::TextureType::Cubemap, 1)
            .pushStage(VK_SHADER_STAGE_VERTEX_BIT)
            .pushConstant(skyboxConstant.get(), 0)
            .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/planets/skybox.vert.spv")
            .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/planets/skybox.frag.spv")
            .build();

        passManager = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig { 2 });

        passManager->initializeRenderPass();

        skyboxModel->update(true, VulkanManager::getInstance()->getSwapchain()->extent, VK_SAMPLE_COUNT_1_BIT, *passManager->getPass(), 0);

        ImGui_ImplVulkan_Init(&init_info, **passManager->getPass());
        // Upload Fonts
        VkTools::immediateExecute([](const VkCommandBuffer& commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); }, VulkanManager::getInstance()->getDevice());

        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    void updateData(int frame) {
        const float elapsed_time = Time::timeSinceStart;
        const vlkx::Camera& cam = camera->getCamera();
        *skyboxConstant->getData<SkyboxTransform>(frame) = { cam.getProjMatrix() * cam.getSkyboxView() };
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

            switch(event.type) {
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            camera->press(vlkx::Camera::Input::Up, Time::deltaTime); break;
                        case SDLK_s:
                            camera->press(vlkx::Camera::Input::Down, Time::deltaTime); break;
                        case SDLK_a:
                            camera->press(vlkx::Camera::Input::Left, Time::deltaTime); break;
                        case SDLK_d:
                            camera->press(vlkx::Camera::Input::Right, Time::deltaTime); break;
                    } break;
                case SDL_MOUSEMOTION:
                    camera->move(event.motion.xrel, event.motion.yrel); break;
                case SDL_QUIT:
                    running = false; break;

            }
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
                            skyboxModel->draw(commands, frame, 1);
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
