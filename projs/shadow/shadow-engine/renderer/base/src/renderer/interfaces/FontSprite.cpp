

#include "renderer/interfaces/FontSprite.h"
#include "shadow/util/string-helpers.h"
namespace rx {

  void FontSprite::FixedUpdate() {

  }

  void FontSprite::Update(float dt) {
      if (IsStatic()) return;

      if (fontResource.IsValid())
          params.style = fontResource.style;
      else
          params.style = 0;
  }

  void FontSprite::Draw(rx::ThreadCommands cmd) const {
      if (IsStatic()) return;

      rx::Font::Draw(text.c_str(), GetTextLength(), params, cmd);
  }

  DirectX::XMFLOAT2 FontSprite::TextSize() const {
      return rx::Font::TextSize(text, params);
  }

  float FontSprite::TextWidth() const {
      return rx::Font::TextWidth(text, params);
  }

  float FontSprite::TextHeight() const {
      return rx::Font::TextHeight(text, params);
  }

  void FontSprite::SetText(std::string &&val) {
      SH::Util::Str::StringConvert(val, text);
  }

  void FontSprite::SetText(const std::string &val) {
      SH::Util::Str::StringConvert(val, text);
  }

  void FontSprite::SetText(std::wstring &&val) {
      text = std::move(val);
  }

  void FontSprite::SetText(const std::wstring &val) {
      text = val;
  }

  const std::string FontSprite::GetText() const {
      std::string ret;
      SH::Util::Str::StringConvert(text, ret);
      return ret;
  }

  const std::wstring &FontSprite::GetTextA() const {
      return text;
  }

  size_t FontSprite::GetTextLength() const {
      return text.length();
  }
}