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

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

namespace ShadowEngine {

    struct AlphaVal {
        alignas(sizeof(float)) float value;
    };

    std::unique_ptr<vlkx::ScreenRenderPassManager> passManager;
    std::unique_ptr<vlkx::RenderCommand> renderCommands;

    std::unique_ptr<vlkx::PerVertexBuffer> vertexBuffer;
    std::unique_ptr<vlkx::PushConstant> alphaPush;

    std::unique_ptr<vlkx::GraphicsPipelineBuilder> builder;
    std::unique_ptr<vlkx::Pipeline> pipeline;


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
        init_info.Subpass = 0;
        init_info.MinImageCount = vk->getSwapchain()->images.size();
        init_info.ImageCount = vk->getSwapchain()->images.size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = VK_NULL_HANDLE;
        init_info.CheckVkResultFn = nullptr;

        const std::array<Geo::VertexColor, 3> vertices {
                Geo::VertexColor { { 0.5,  -0.5, 0 },  { 1, 0, 0 } },
                Geo::VertexColor { { 0,     0.5, 0 },  { 0, 0, 1 } },
                Geo::VertexColor { { -0.5, -0.5, 0 },  { 0, 1, 0 } }
        };

        const vlkx::PerVertexBuffer::NoIndexBufferMeta meta {
                { { vlkx::PerVertexBuffer::VertexDataMeta { vertices } } }
        };

        vertexBuffer = std::make_unique<vlkx::StaticPerVertexBuffer>( meta, Geo::VertexColor::getAttributeDesc() );

        alphaPush = std::make_unique<vlkx::PushConstant>(sizeof(AlphaVal), 2);

        builder = std::make_unique<vlkx::GraphicsPipelineBuilder>();
        (*builder)
            .name("Triangle Render")
            .addVertex(0, Geo::VertexColor::getBindingDesc(), vertexBuffer->getAttrs(0))
            .layout({}, { alphaPush->makeRange(VK_SHADER_STAGE_FRAGMENT_BIT) })
            .colorBlend({ vlkx::Pipeline::getAlphaBlendState(true) })
            .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/tri/tri.vert.spv")
            .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/tri/tri.frag.spv");

        passManager = std::make_unique<vlkx::ScreenRenderPassManager>(vlkx::RendererConfig());

        passManager->initializeRenderPass();

        (*builder)
            .multiSample(VK_SAMPLE_COUNT_1_BIT)
            .viewport( { { 0, 0, static_cast<float>(window_->Width), static_cast<float>(window_->Height), 0, 1 }, { { 0, 0 }, VulkanManager::getInstance()->getSwapchain()->extent } })
            .renderPass(**passManager->getPass(), 0);

        pipeline = builder->build();

        ImGui_ImplVulkan_Init(&init_info, **passManager->getPass());
        // Upload Fonts
        VkTools::immediateExecute([](const VkCommandBuffer& commands) { ImGui_ImplVulkan_CreateFontsTexture(commands); }, VulkanManager::getInstance()->getDevice());

    }

    void updateData(int frame) {
        alphaPush->getData<AlphaVal>(frame)->value = glm::abs(glm::sin(Time::timeSinceStart));
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
                        [&frame](const VkCommandBuffer& commands) {
                            pipeline->bind(commands);
                            alphaPush->upload(commands, pipeline->getLayout(), frame, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
                            vertexBuffer->draw(commands, 0, 0, 1);

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
