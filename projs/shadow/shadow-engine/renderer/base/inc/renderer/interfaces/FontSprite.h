#pragma once

#include <string>
#include "FontSprite.h"
#include "Font.h"

namespace rx {

  // A string, in a certain font, as a sprite.
  class FontSprite {
  private:
    // Behavioural flags.
    enum Flags {
      EMPTY = 0,
      HIDDEN = 1 << 0, // Do not show on screen
      STATIC = 1 << 1, // Do not move
      BILLBOARD = 1 << 2, // Rotate to always face the camera
      SIZE_STATIC = 1 << 3 // Do not change size as the camera moves away
    };
    uint32_t flags = Flags::EMPTY;

  public:
    // The content of the actual text being displayed.
    std::wstring text;
    // The style information of the font contained.
    rx::Font::RenderMode params;
    // The resource of the font being used.
    ShadowEngine::Resource fontResource;
    // The filename of the font being used.
    std::string fontName;

    FontSprite() = default;
    FontSprite(const std::string& text, const rx::Font::RenderMode& params = rx::Font::RenderMode(), const std::string& fontName = "")
        : params(params), fontName(fontName) {
        SetText(text);
        // If we were given a font to use, load it.
        if (!fontName.empty())
            fontResource = SH::ShadowApplication::Get().engine->getResourceManager().load<rx::Font::FontResource>(ShadowEngine::Path(fontName));
    }

    virtual ~FontSprite() = default;

    // Update at a fixed interval; ie. desynchronized from frames.
    virtual void FixedUpdate();
    // Update with a frame.
    virtual void Update(float dt);
    // Draw the font to the screen with the given batch.
    virtual void Draw(rx::ThreadCommands cmd) const;

    // A helper to define a bitwise flag setter.
    #define SET(x, y) constexpr void Set##x(bool val = true) { if (val) flags |= (y); else flags &= ~(y); }
    // A helper to define a bitwise flag getter.
    #define GET(x, y) constexpr bool Get##x() const { return flags & (y); }

    SET(Hidden, HIDDEN);
    SET(Static, STATIC);
    SET(Billboard, BILLBOARD);
    SET(SizeStatic, SIZE_STATIC);

    GET(Hidden, HIDDEN);
    GET(Static, STATIC);
    GET(Billboad, BILLBOARD);
    GET(SizeStatic, SIZE_STATIC);

    // Get the 2 dimensional size of a single letter of the text.
    DirectX::XMFLOAT2 TextSize() const;
    // Get the width of a single character in the text.
    float TextWidth() const;
    // Get the height of a single character in the text.
    float TextHeight() const;

    // Set the text to a new value.
    void SetText(const std::string& val);
    void SetText(std::string&& val);
    void SetText(const std::wstring& val);
    void SetText(std::wstring&& val);

    // Get the text being displayed.
    const std::string& GetText() const;
    const std::wstring& GetTextA() const;

    // Get the overall length of the text, in pixels in screenspace.
    size_t GetTextLength() const;

    // TODO: Animations?
  };
}