
#include <string>
#include "renderer/interfaces/Sprite.h"
#include "shadow/core/ShadowApplication.h"
namespace rx {
  Sprite::Sprite(const std::string& newTex, const std::string& mask) {
      if (!newTex.empty()) {
          textureName = newTex;
          textureResource = SH::ShadowApplication::Get().engine->getResourceManager().load<Texture>(newTex);
      }
      if (!mask.empty()) {
          maskName = mask;
          maskResource = SH::ShadowApplication::Get().engine->getResourceManager().load<Texture>(mask);
          params.setMaskMap(&maskResource);
      }
  }

  void Sprite::Draw(rx::ThreadCommands cmd) const {
      if (IsHidden()) return;

      rx::Image::Draw(GetTexture(), params, cmd);
  }

  void Sprite::FixedUpdate() {

  }

  void Sprite::Update(float dt) {

  }



}