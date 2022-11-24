#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_vulkan.h"
#include "core/SDL2Module.h"
#include "imgui_impl_sdl.h"
#include "core/Time.h"
#include "vlkx/render/Camera.h"

#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

SHObject_Base_Impl(GameModule)

std::unique_ptr<vlkx::UserPerspectiveCamera> camera;

void GameModule::PreInit() { spdlog::info("Game Module loading.."); }

void GameModule::Init() {
}

void GameModule::Update(int frame) {
}

void GameModule::Render(VkCommandBuffer& commands, int frame) {

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