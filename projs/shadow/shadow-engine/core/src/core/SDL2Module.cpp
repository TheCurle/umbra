//
// Created by dpete on 30/08/2022.
//

#include "core/SDL2Module.h"
#include "core/ShadowWindow.h"
#include "spdlog/spdlog.h"
#include "imgui_impl_sdl.h"

SHObject_Base_Impl(ShadowEngine::SDL2Module)

void ShadowEngine::SDL2Module::Init() {
}

void ShadowEngine::SDL2Module::PreInit() {
    // Initialize SDL. SDL_Init will return -1 if it fails.
    if ( SDL_Init( SDL_INIT_EVERYTHING ) < 0 ) {
        spdlog::error("Error creating window: " + std::string(SDL_GetError()));
        //system("pause");
        // End the program
        //return 1;
    }

    _window = new ShadowWindow(1280,720);
    SDL_SetWindowResizable(_window->sdlWindowPtr, SDL_TRUE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);
}

void ShadowEngine::SDL2Module::Update(int frame) {}

void ShadowEngine::SDL2Module::Recreate() {}

void ShadowEngine::SDL2Module::Render(VkCommandBuffer& commands, int frame) {}

void ShadowEngine::SDL2Module::OverlayRender() {}

void ShadowEngine::SDL2Module::LateRender(VkCommandBuffer& commands, int frame) {}

std::string ShadowEngine::SDL2Module::GetName() {
    return this->GetType();
}

void ShadowEngine::SDL2Module::AfterFrameEnd() {

}

void ShadowEngine::SDL2Module::Event(SDL_Event *e) {
    ImGui_ImplSDL2_ProcessEvent(e);
}

void ShadowEngine::SDL2Module::Destroy() {
    SDL_DestroyWindow(_window->sdlWindowPtr);
    SDL_Quit();
}

void ShadowEngine::SDL2Module::PreRender() {
}