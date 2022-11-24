#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
#include "core/SDL2Module.h"
#include "imgui_impl_sdl.h"
#include "core/Time.h"
#include "vlkx/render/Camera.h"
#include "vlkx/vulkan/abstraction/Buffer.h"
#include "temp/model/Builder.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

struct Transformation {
    alignas(sizeof(glm::mat4)) glm::mat4 proj_view_model;
};

std::unique_ptr<vlkx::PushConstant> trans_constant_;
std::unique_ptr<vlkxtemp::Model> cube_model_;
float aspectRatio;

void GameModule::PreInit() {  }

void GameModule::Init() {
    spdlog::info("Game Module loading level..");
    trans_constant_ = std::make_unique<vlkx::PushConstant>(
            sizeof(Transformation), 2);

    auto extent = VulkanModule::getInstance()->getSwapchain()->extent;
    aspectRatio = (float) extent.width / extent.height;

    /* Model */
    cube_model_ = vlkxtemp::ModelBuilder {
            "Walrus", 2, aspectRatio,
            vlkxtemp::ModelBuilder::SingleMeshModel {"resources/walrus/walrus.obj", 1,
                         {{ vlkxtemp::ModelBuilder::TextureType::Diffuse, { { "resources/walrus/texture.png" } } } }
            }}
            .bindTextures(vlkxtemp::ModelBuilder::TextureType::Diffuse, 1)
            .pushStage(VK_SHADER_STAGE_VERTEX_BIT)
            .pushConstant(trans_constant_.get(), 0)
            .shader(VK_SHADER_STAGE_VERTEX_BIT, "resources/walrus/cube.vert.spv")
            .shader(VK_SHADER_STAGE_FRAGMENT_BIT, "resources/walrus/cube.frag.spv")
            .build();

}

void GameModule::Update(int frame) {
    const float elapsed_time = Time::timeSinceStart;
    const glm::mat4 model = glm::rotate(glm::mat4{1.0f},
                                        elapsed_time * glm::radians(90.0f),
                                        glm::vec3{1.0f, 1.0f, 0.0f});
    const glm::mat4 view = glm::lookAt(glm::vec3{3.0f}, glm::vec3{0.0f},
                                       glm::vec3{0.0f, 0.0f, 1.0f});
    const glm::mat4 proj = glm::perspective(
            glm::radians(45.0f), aspectRatio,
            0.1f, 100.0f);
    *trans_constant_->getData<Transformation>(frame) = {proj * view * model};
}

void GameModule::Render(VkCommandBuffer& commands, int frame) {
    cube_model_->draw(commands, frame, 1);
}

void GameModule::OverlayRender() {
    bool active = true;
    ImGui::Begin("Game module window", &active, ImGuiWindowFlags_MenuBar);
    ImGui::Text("Such text from curle's branch");
    ImGui::End();

    bool showDemo = true;
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);
}

void GameModule::AfterFrameEnd() {
    Time::UpdateTime();
}

void GameModule::LateRender(VkCommandBuffer& commands, int frame) {}
void GameModule::PreRender() {}

void GameModule::Destroy() {}
void GameModule::Event(SDL_Event *) {}