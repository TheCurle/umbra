#include <GameModule.h>
#include "imgui.h"
#include "spdlog/spdlog.h"
#include "vlkx/render/geometry/SingleRenderer.h"
#include "imgui_impl_vulkan.h"
#include "core/ShadowApplication.h"
#include "core/SDL2Module.h"
#include "imgui_impl_sdl.h"
#include "core/Time.h"


#define CATCH(x) \
    try { x } catch (std::exception& e) { spdlog::error(e.what()); exit(0); }

// Create the renderer
SingleRenderer object;

// Create the camera
Camera camera;

SHObject_Base_Impl(GameModule)

void GameModule::PreInit() { spdlog::info("Game Module loading.."); }

void GameModule::Init() {
    CATCH(object.createSingleRenderer(Geo::MeshType::Cube, glm::vec3(-1, 0, -1), glm::vec3(0.5));)
    camera.init(45, 1280, 720, 0.1f, 10000.0f);
    camera.setPosition(glm::vec3(0, 0, 4));
}

void GameModule::Update() {
    object.setRotation(glm::rotate(object.getRotation(), (float) 0.5, glm::vec3(1, 0, 0)));
}

void GameModule::Render() {
    bool active = true;
    ImGui::Begin("Game module window", &active, ImGuiWindowFlags_MenuBar);
    ImGui::Text("Such text from curle's branch");
    ImGui::End();

    CATCH(object.updateUniforms(camera);)
    CATCH(object.draw();)

    bool showDemo = true;
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);
}

void GameModule::AfterFrameEnd() {
    Time::UpdateTime();
}

void GameModule::LateRender() {}
void GameModule::PreRender() {}


void GameModule::Destroy() {}
void GameModule::Event(SDL_Event *) {}