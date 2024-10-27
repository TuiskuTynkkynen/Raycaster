#include "UI.h"
#include "UI.h"

#include "Layout.h"
#include "Internal.h"
#include "Widgets.h"

#include "Core/Base/Input.h"
#include "Core/Debug/Debug.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

namespace Core {
    void UI::Init() {
        RC_ASSERT(!UI::Internal::System, "UI has already been initialized");

        UI::Internal::System = std::make_unique<Internal::UISystem>();
    }
    
    void UI::Shutdown() {
        RC_ASSERT(UI::Internal::System, "UI has not been initialized");

        UI::Internal::System.reset();
    }

    void UI::Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout) {
        RC_ASSERT(UI::Internal::System, "UI has not been initialized");
        UI::Internal::System->Elements.clear();

        UI::Internal::System->Position = screenPosition;
        UI::Internal::System->Size = screenSize;

        UI::Internal::System->Elements.emplace_back(SurfaceType::None, layout, PositioningType::Auto, glm::vec2(0.5f), glm::vec2(1.0f), std::array<glm::vec4, 3>{glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)});
    }

    void UI::Update() {
        RC_ASSERT(UI::Internal::System, "Tried to update UI before initializing");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to update UI before calling UI Begin");

        float mouseX = Input::GetMouseX() / UI::Internal::System->Size.x;
        float mouseY = Input::GetMouseY() / UI::Internal::System->Size.y;
        UI::Internal::System->HoverID = 0;
        if (!Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
            UI::Internal::System->ActiveID = 0;
        }

        size_t lastParentId = -1;
        std::unique_ptr<Layout> layout = std::make_unique<NoLayout>(UI::Internal::System->Elements.front());
        for (size_t i = 1; i < UI::Internal::System->Elements.size(); i++) {
            Surface& current = UI::Internal::System->Elements[i];
            const Surface& parent = UI::Internal::System->Elements[current.ParentID];
            
            if (current.ParentID != lastParentId) { 
                switch (parent.Layout) {
                case Core::UI::LayoutType::None:
                    layout = std::make_unique<NoLayout>(parent);
                    break;
                case Core::UI::LayoutType::Vertical:
                case Core::UI::LayoutType::Horizontal:
                    layout = std::make_unique<LinearLayout>(i);
                    break;
                }

                lastParentId = current.ParentID;
            }

            layout->Next(current);
            if (current.Widget) {
                current.Widget->Update(current);
            }

            if (current.Type == SurfaceType::Button && (!UI::Internal::System->ActiveID || UI::Internal::System->ActiveID == i)) {
                if (mouseX <= current.Position.x + current.Size.x * 0.5f && mouseX >= current.Position.x - current.Size.x * 0.5f
                    && mouseY <= current.Position.y + current.Size.y * 0.5f && mouseY >= current.Position.y - current.Size.y * 0.5f) {

                    UI::Internal::System->HoverID = i;

                    if (Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
                        UI::Internal::System->ActiveID = i;
                    }
                }
            }
        }
    }

    void UI::Render() {
        RC_ASSERT(UI::Internal::System, "Tried to render UI before initializing");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to render UI before calling UI Begin");

        RenderAPI::SetViewPort(UI::Internal::System->Position.x, UI::Internal::System->Position.y, UI::Internal::System->Position.x + UI::Internal::System->Size.x, UI::Internal::System->Position.y + UI::Internal::System->Size.y);
        Renderer2D::BeginScene(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f));
        
        if(Internal::Font) { Internal::Font->ActivateAtlas(2); }
        if(Internal::TextureAtlas) { Internal::TextureAtlas->Activate(3); }

        for (size_t i = 1; i < UI::Internal::System->Elements.size(); i++) {
            Surface& s = UI::Internal::System->Elements[i];
            
            if (s.Widget && s.Widget->Render(s)) {
                continue;
            }

            uint32_t colourIndex = UI::Internal::System->ActiveID == i ? 2 : UI::Internal::System->HoverID == i ? 1 : 0;
            glm::vec4& colour = s.Colours[colourIndex];

            if (s.Size.x * s.Size.y == 0.0f) { continue; }

            Renderer2D::DrawFlatQuad({ s.Position.x, s.Position.y, 0.0f }, { s.Size.x, s.Size.y, 0.0f }, colour);
        }

        Renderer2D::EndScene();
    }

    void UI::End() {
        Update();
        Render();

        UI::Internal::System->Elements.clear();
    }

    void UI::BeginContainer(PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, LayoutType layout) {
        RC_ASSERT(UI::Internal::System, "Tried to begin a UI container before initializing UI");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to begin a UI container before calling UI Begin");

        UI::Internal::System->Elements.emplace_back(SurfaceType::None, layout, positioning, position, size * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour }, UI::Internal::System->OpenElement);

        UI::Internal::System->Elements[UI::Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = UI::Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > UI::Internal::System->OpenElement; i--) {
            if (UI::Internal::System->Elements[i].ParentID == UI::Internal::System->OpenElement) {
                UI::Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        UI::Internal::System->OpenElement = currentIndex;
    }

    void UI::EndContainer() {
        size_t parentId = UI::Internal::System->Elements[UI::Internal::System->OpenElement].ParentID;
        RC_ASSERT(parentId != -1, "Tried to end a UI container before begining one");
        
        UI::Internal::System->OpenElement = parentId;
    }

    template <typename T>
    bool UI::Button(std::basic_string_view<T> text, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& buttonColours, const std::array<glm::vec4, 3>& textColours) {
        bool result = Button(positioning, position, size, buttonColours[0], buttonColours[1], buttonColours[2]);

        size_t open = Internal::System->OpenElement;
        Internal::System->OpenElement = UI::Internal::System->Elements.size() - 1;
        Text(text, glm::vec2(0.9f), textColours[0], textColours[1], textColours[2]);
        UI::Internal::System->Elements[Internal::System->OpenElement].ChildCount++;
        Internal::System->OpenElement = open;
        
        return result;
    }
    template bool UI::Button<char>(std::string_view, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template bool UI::Button<wchar_t>(std::wstring_view, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);

    bool UI::Button(PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(UI::Internal::System, "Tried to create a UI button before initializing UI");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to create a UI button before calling UI Begin");

        UI::Internal::System->Elements.emplace_back(SurfaceType::Button, LayoutType::None, positioning, position, size * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, UI::Internal::System->OpenElement);
        
        UI::Internal::System->Elements[UI::Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = UI::Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > UI::Internal::System->OpenElement; i--) {
            if (UI::Internal::System->Elements[i].ParentID == UI::Internal::System->OpenElement) {
                UI::Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        return Input::IsButtonReleased(RC_MOUSE_BUTTON_LEFT) && UI::Internal::System->HoverID == currentIndex && UI::Internal::System->ActiveID == currentIndex;
    }

    bool UI::TextureButton(glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        bool result = Button(positioning, position, size, primaryColour, hoverColour, activeColour);

        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasIndices, atlasSize);

        return result;
    }

    template <typename T>
    bool UI::TextureButton(std::basic_string_view<T> text, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& buttonColours, const std::array<glm::vec4, 3>& textColours) {
        bool result = Button(positioning, position, size, buttonColours[0], buttonColours[1], buttonColours[2]);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasIndices, atlasSize);

        size_t open = Internal::System->OpenElement;
        Internal::System->OpenElement = UI::Internal::System->Elements.size() - 1;
        Text(text, glm::vec2(0.9f), textColours[0], textColours[1], textColours[2]);
        UI::Internal::System->Elements[Internal::System->OpenElement].ChildCount++;
        Internal::System->OpenElement = open;

        return result;
    }
    template bool UI::TextureButton<char>(std::string_view, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template bool UI::TextureButton<wchar_t>(std::wstring_view, glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);

    void UI::Texture(glm::uvec3 atlasIndices, glm::vec2 atlasSize, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour) {
        RC_ASSERT(UI::Internal::System, "Tried to create a UI texture before initializing UI");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to create a UI texture before calling UI Begin");

        UI::Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ colour, colour, colour }, UI::Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasIndices, atlasSize);

        UI::Internal::System->Elements[UI::Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = UI::Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > UI::Internal::System->OpenElement; i--) {
            if (UI::Internal::System->Elements[i].ParentID == UI::Internal::System->OpenElement) {
                UI::Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

    }


    void UI::Text(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(UI::Internal::System, "Tried to create a UI text before initializing UI");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to create a UI text before calling UI Begin");

        UI::Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, UI::Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique < Widgets::TextWidget<char>>(text);

        UI::Internal::System->Elements[UI::Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = UI::Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > UI::Internal::System->OpenElement; i--) {
            if (UI::Internal::System->Elements[i].ParentID == UI::Internal::System->OpenElement) {
                UI::Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }

    void UI::Text(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(UI::Internal::System, "Tried to create a UI text before initializing UI");
        RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to create a UI text before calling UI Begin");

        UI::Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ glm::vec4(1.0f), glm::vec4(0.8f), glm::vec4(0.5f) }, UI::Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique < Widgets::TextWidget<wchar_t>>(text);

        UI::Internal::System->Elements[UI::Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = UI::Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > UI::Internal::System->OpenElement; i--) {
            if (UI::Internal::System->Elements[i].ParentID == UI::Internal::System->OpenElement) {
                UI::Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }

    void UI::SetFont(std::shared_ptr<Core::Font> font) { 
        Internal::Font = font; 
    }


    void UI::SetTextureAtlas(std::shared_ptr<Core::Texture2D> atlas, glm::uvec2 atlasSize) {
        Internal::TextureAtlas = atlas;
        Internal::AtlasSize = atlasSize;
    }
}