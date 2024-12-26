#include "UI.h"

#include "Layout.h"
#include "Internal.h"
#include "Widgets.h"

#include "Core/Base/Input.h"
#include "Core/Debug/Debug.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

bool AABB(glm::vec2 mousePosition, glm::vec2 position, glm::vec2 size) {
    return (mousePosition.x <= position.x + glm::abs(size.x) * 0.5f && mousePosition.x >= position.x - glm::abs(size.x) * 0.5f
        && mousePosition.y <= position.y + glm::abs(size.y) * 0.5f && mousePosition.y >= position.y - glm::abs(size.y) * 0.5f);
}

namespace Core {
    void UI::Init() {
        RC_ASSERT(!Internal::System, "UI has already been initialized");

        Internal::System = std::make_unique<Internal::UISystem>();
        Internal::Input = std::make_unique<Internal::UIInputState>();
    }
    
    void UI::Shutdown() {
        RC_ASSERT(Internal::System, "UI has not been initialized");

        Internal::System.reset();
        Internal::Input.reset();
    }

    void UI::Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout, const glm::vec4& colour) {
        RC_ASSERT(Internal::System, "UI has not been initialized");
        Internal::System->Elements.clear();

        Internal::System->Position = screenPosition;
        Internal::System->Size = screenSize;

        Internal::System->Elements.emplace_back(SurfaceType::None, layout, PositioningType::Auto, glm::vec2(0.5f), glm::vec2(1.0f), std::array<glm::vec4, 3>{colour, colour, colour});
        Internal::System->OpenElement = 0;
    }

    void UI::Update() {
        RC_ASSERT(Internal::System, "Tried to update UI before initializing");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to update UI before calling UI Begin");

        bool isCaptured = Internal::System->Elements[Internal::System->ActiveID].Type >= SurfaceType::Capture;

        Internal::System->HoverID = 0;
        if (Internal::Input->MouseState.Left == Internal::MouseButtonState::None && !isCaptured) {
            Internal::System->ActiveID = 0;
        }
        
        Internal::Input->MouseState.Left = Internal::MouseButtonState(Internal::Input->MouseState.Left == Internal::MouseButtonState::Held);
        Internal::Input->MouseState.Right = Internal::MouseButtonState(Internal::Input->MouseState.Right == Internal::MouseButtonState::Held);

        size_t lastParentId = -1;
        std::unique_ptr<Layout> layout = std::make_unique<NoLayout>(Internal::System->Elements.front());
        for (size_t i = 1; i < Internal::System->Elements.size(); i++) {
            Surface& current = Internal::System->Elements[i];
            const Surface& parent = Internal::System->Elements[current.ParentID];
            
            if (current.ParentID != lastParentId) { 
                switch (parent.Layout) {
                case LayoutType::None:
                    layout = std::make_unique<NoLayout>(parent);
                    break;
                case LayoutType::Vertical:
                case LayoutType::Horizontal:
                    layout = std::make_unique<LinearLayout>(i);
                    break;
                case LayoutType::Crop:
                    layout = std::make_unique<CropLayout<NoLayout>>(NoLayout(parent));
                    break;
                case LayoutType::CropVertical:
                case LayoutType::CropHorizontal:
                    layout = std::make_unique<CropLayout<LinearLayout>>(LinearLayout(i));
                    break;
                default:
                    RC_WARN("Invalid LayoutType");
                }

                lastParentId = current.ParentID;
            }

            layout->Next(current);

            if (current.Type >= SurfaceType::Hoverable && (!Internal::System->ActiveID || Internal::System->ActiveID == i || isCaptured)) {
                bool skip = false;
                if (parent.Type >= SurfaceType::Capture && current.ParentID == Internal::System->HoverID) {
                    skip = true;
                }
                
                if (!skip && parent.Layout >= LayoutType::Crop && !AABB(Internal::Input->MouseState.Position, parent.Position, parent.Size)) {
                    skip = true;
                }

                if (!skip && AABB(Internal::Input->MouseState.Position, current.Position, current.Size)) {
                    Internal::System->HoverID = i;
                }
            }

            if (current.Widget) {
                current.Widget->Update(current);
            }
        }

        Internal::Input->MouseState.ScrollOffset = 0.0f;

        Internal::Input->KeyboardState.InputedText.clear();
        Internal::Input->KeyboardState.SpecialKeys.reset();
    }

    void UI::Render() {
        RC_ASSERT(Internal::System, "Tried to render UI before initializing");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to render UI before calling UI Begin");

        RenderAPI::SetViewPort(Internal::System->Position.x, Internal::System->Position.y, Internal::System->Position.x + Internal::System->Size.x, Internal::System->Position.y + Internal::System->Size.y);
        Renderer2D::BeginScene(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f));
        
        if(Internal::Font) { Internal::Font->ActivateAtlas(2); }
        if(Internal::TextureAtlas) { Internal::TextureAtlas->Activate(3); }
        
        size_t scissorID = 0;
        std::vector<size_t> scissorIDs;
        for (size_t i = 0; i < Internal::System->Elements.size(); i++) {
            Surface& s = Internal::System->Elements[i];

            {
                //Remove unneeded elments
                while (!scissorIDs.empty() && scissorIDs.back() >= s.ParentID) {
                    scissorIDs.pop_back();
                }

                //Insert current elements parent if a scissor test based on it is needed
                if (Internal::System->Elements[s.ParentID].Layout >= LayoutType::Crop) {
                    scissorIDs.push_back(s.ParentID);
                }

                //Set scissor test parameters to scissorIDs.back() position and size if not already set
                if (!scissorIDs.empty() && scissorIDs.back() != scissorID) {
                    scissorID = scissorIDs.back();
                    Surface& parent = Internal::System->Elements[scissorID];

                    float offsetX = glm::round(Internal::System->Size.x * (parent.Position.x - parent.Size.x * 0.5f));
                    float offsetY = glm::round(Internal::System->Size.y * (1.0f - parent.Position.y - parent.Size.y * 0.5f));
                    glm::uvec2 size = glm::round(Internal::System->Size * parent.Size);

                    Renderer2D::Flush();

                    RenderAPI::SetScissor(true);
                    RenderAPI::SetScissorRectangle(offsetX, offsetY, size.x, size.y);
                }

                //Remove scissor test if it is not needed
                if (scissorIDs.empty() && scissorID) {
                    scissorID = 0;

                    Renderer2D::Flush();
                    RenderAPI::SetScissor(false);
                }
            }

            if (s.Widget && s.Widget->Render(s)) {
                continue;
            }

            uint32_t colourIndex = Internal::System->ActiveID == i ? 2 : Internal::System->HoverID == i ? 1 : 0;
            glm::vec4& colour = s.Colours[colourIndex];

            if (s.Size.x * s.Size.y == 0.0f || colour.a == 0.0f) { continue; }

            if (s.Type == SurfaceType::None) {
                Renderer2D::DrawFlatShapeQuad({ s.Position.x, s.Position.y, 0.0f }, { s.Size.x, s.Size.y, 0.0f },  colour);
                continue;
            } 
            
            Renderer2D::DrawFlatRoundedQuad(s.Size * 0.99f, 0.2f, 2, { s.Position.x, s.Position.y, 0.0f }, glm::vec3(1.0f), colour);
            Renderer2D::DrawFlatRoundedQuadEdge(s.Size, 0.075f, 0.2f, 5, { s.Position.x, s.Position.y, 0.0f }, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });
        }

        Renderer2D::EndScene();
        RenderAPI::SetScissor(false);
    }

    void UI::End() {
        Update();
        Render();
    }

    void UI::BeginContainer(PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, LayoutType layout) {
        RC_ASSERT(Internal::System, "Tried to begin a UI container before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to begin a UI container before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::None, layout, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, primaryColour, primaryColour }, Internal::System->OpenElement);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        Internal::System->OpenElement = currentIndex;
    }

    void UI::EndContainer() {
        size_t parentId = Internal::System->Elements[Internal::System->OpenElement].ParentID;
        RC_ASSERT(parentId != -1, "Tried to end a UI container before begining one");
        
        Internal::System->OpenElement = parentId;
    }

    void UI::BeginScrollContainer(float& offset, PositioningType positioning, glm::vec2 position, glm::vec2 size, bool vertical, float speed, const glm::vec4& primaryColour, const glm::vec4& hoverColour) {
        RC_ASSERT(Internal::System, "Tried to begin a UI scroll container before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to begin a UI scroll container before calling UI Begin");

        LayoutType layout = vertical ? LayoutType::CropVertical : LayoutType::CropHorizontal;
        Internal::System->Elements.emplace_back(SurfaceType::Hoverable, layout, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{primaryColour, hoverColour}, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::ScrollWidget>(offset, vertical, speed);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        Internal::System->OpenElement = currentIndex;
    }

    template <typename T>
    bool UI::Button(std::basic_string_view<T> text, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& buttonColours, const std::array<glm::vec4, 3>& textColours) {
        bool result = Button(positioning, position, size, buttonColours[0], buttonColours[1], buttonColours[2]);

        size_t open = Internal::System->OpenElement;
        Internal::System->OpenElement = Internal::System->Elements.size() - 1;
        Text(text, glm::vec2(0.9f), textColours[0], textColours[1], textColours[2]);
        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;
        Internal::System->OpenElement = open;
        
        return result;
    }

    template bool UI::Button<char>(std::string_view, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template bool UI::Button<wchar_t>(std::wstring_view, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);

    bool UI::Button(PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(Internal::System, "Tried to create a UI button before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI button before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::Button, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, Internal::System->OpenElement);
        
        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        return Internal::Input->MouseState.Left == Internal::MouseButtonState::Released && Internal::System->HoverID == currentIndex && Internal::System->ActiveID == currentIndex;
    }

    bool UI::TextureButton(const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        bool result = Button(positioning, position, size, primaryColour, hoverColour, activeColour);

        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasProperties.Indices, atlasProperties.Size);

        return result;
    }

    template <typename T>
    bool UI::TextureButton(std::basic_string_view<T> text, const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& buttonColours, const std::array<glm::vec4, 3>& textColours) {
        bool result = Button(positioning, position, size, buttonColours[0], buttonColours[1], buttonColours[2]);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasProperties.Indices, atlasProperties.Size);

        size_t open = Internal::System->OpenElement;
        Internal::System->OpenElement = Internal::System->Elements.size() - 1;
        Text(text, glm::vec2(0.9f), textColours[0], textColours[1], textColours[2]);
        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;
        Internal::System->OpenElement = open;

        return result;
    }

    template bool UI::TextureButton<char>(std::string_view, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template bool UI::TextureButton<wchar_t>(std::wstring_view, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);

    void UI::Texture(const AtlasProperties& atlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& colour) {
        RC_ASSERT(Internal::System, "Tried to create a UI texture before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI texture before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ colour, colour, colour }, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureWidget>(atlasProperties.Indices, atlasProperties.Size);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

    }

    void UI::Text(std::string_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(Internal::System, "Tried to create a UI text before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI text before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique < Widgets::TextWidget<char>>(text);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }

    void UI::Text(std::wstring_view text, PositioningType positioning, glm::vec2 position, glm::vec2 relativeSize, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(Internal::System, "Tried to create a UI text before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI text before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, positioning, position, relativeSize * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ glm::vec4(1.0f), glm::vec4(0.8f), glm::vec4(0.5f) }, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique < Widgets::TextWidget<wchar_t>>(text);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }

    void UI::TextInputField(std::vector<char>& text, std::string_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& boxColours, const std::array<glm::vec4, 3>& highlightColours, const std::array<glm::vec4, 3>& textColours, const std::array<glm::vec4, 3>& highlightedTextColours) {
        RC_ASSERT(Internal::System, "Tried to create a UI text input field before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI text input field before calling UI Begin");
        RC_ASSERT(Internal::Font, "Tried to create a UI text input field before setting UI font");
        
        Internal::System->Elements.emplace_back(SurfaceType::TextInput, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, boxColours, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::TextInputWidget<char>>(text, selectionStart, selectionEnd, highlightColours, highlightedTextColours);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        size_t parentIndex = Internal::System->OpenElement;

        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        Internal::System->OpenElement = currentIndex;

        BeginScrollContainer(scrollOffset, glm::vec2(1.0f), false, 1.0f, glm::vec4(0.0f), glm::vec4(0.0f));
        {
            Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, PositioningType::Auto, glm::vec2(0.0f), Internal::System->Elements[Internal::System->OpenElement].Size, textColours, Internal::System->OpenElement);
            Internal::System->Elements.back().Widget = std::make_unique<Widgets::TextDisplayWidget<char>>(label, textScale);
            Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

            size_t currentIndex = Internal::System->Elements.size() - 1;
            for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
                if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                    Internal::System->Elements[i].SiblingID = currentIndex;
                    break;
                }
            }
        }
        EndScrollContainer();
        
        Internal::System->OpenElement = parentIndex;
    }
    
    void UI::TextInputField(std::vector<wchar_t>& text, std::wstring_view label, float textScale, float& scrollOffset, size_t& selectionStart, size_t& selectionEnd, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& boxColours, const std::array<glm::vec4, 3>& highlightColours, const std::array<glm::vec4, 3>& textColours, const std::array<glm::vec4, 3>& highlightedTextColours) {
        RC_ASSERT(Internal::System, "Tried to create a UI text input field before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI text input field before calling UI Begin");
        RC_ASSERT(Internal::Font, "Tried to create a UI text input field before setting UI font");
        
        Internal::System->Elements.emplace_back(SurfaceType::TextInput, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, boxColours, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::TextInputWidget<wchar_t>>(text, selectionStart, selectionEnd, highlightColours, highlightedTextColours);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        size_t parentIndex = Internal::System->OpenElement;

        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        Internal::System->OpenElement = currentIndex;

        BeginScrollContainer(scrollOffset, glm::vec2(1.0f), false, 1.0f, glm::vec4(0.0f), glm::vec4(0.0f));
        {
            Internal::System->Elements.emplace_back(SurfaceType::None, LayoutType::None, PositioningType::Auto, glm::vec2(0.0f), Internal::System->Elements[Internal::System->OpenElement].Size, textColours, Internal::System->OpenElement);
            Internal::System->Elements.back().Widget = std::make_unique<Widgets::TextDisplayWidget<wchar_t>>(label, textScale);
            Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

            size_t currentIndex = Internal::System->Elements.size() - 1;
            for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
                if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                    Internal::System->Elements[i].SiblingID = currentIndex;
                    break;
                }
            }
        }
        EndScrollContainer();
        
        Internal::System->OpenElement = parentIndex;
    }

    void UI::Toggle(bool& enabled, PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(Internal::System, "Tried to create a UI toggle before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI toggle before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::Activatable, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::ToggleWidget>(enabled);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        enabled ^= Internal::Input->MouseState.Left == Internal::MouseButtonState::Released && Internal::System->HoverID == currentIndex && Internal::System->ActiveID == currentIndex;
    }
    
    void UI::TextureToggle(bool& enabled, const AtlasProperties& boxAtlasProperties, const glm::uvec3& checkAtlasIndices, PositioningType positioning, glm::vec2 position, glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
        RC_ASSERT(Internal::System, "Tried to create a UI toggle before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI toggle before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::Activatable, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, std::array<glm::vec4, 3>{ primaryColour, hoverColour, activeColour }, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureToggleWidget>(enabled, boxAtlasProperties.Indices, checkAtlasIndices, boxAtlasProperties.Size);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }

        enabled ^= Internal::Input->MouseState.Left == Internal::MouseButtonState::Released && Internal::System->HoverID == currentIndex && Internal::System->ActiveID == currentIndex;
    }

    template <typename T>
    void UI::Slider(T& value, T min, T max, bool vertical, float sliderSize, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& sliderColours, const std::array<glm::vec4, 3>& boxColours) {
        RC_ASSERT(Internal::System, "Tried to create a UI slider before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI slider before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::Activatable, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, boxColours, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::SliderWidget<T>>(value, min, max, vertical, sliderSize, sliderColours);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }

    template void UI::Slider<int8_t>(int8_t&, int8_t, int8_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<uint8_t>(uint8_t&, uint8_t, uint8_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<int32_t>(int32_t&, int32_t, int32_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<uint32_t>(uint32_t&, uint32_t, uint32_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<int64_t>(int64_t&, int64_t, int64_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<uint64_t>(uint64_t&, uint64_t, uint64_t, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<float>(float&, float, float, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);
    template void UI::Slider<double>(double&, double, double, bool, float, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&, const std::array<glm::vec4, 3>&);

    template <typename T>
    void UI::TextureSlider(T& value, T min, T max, bool vertical, const AtlasProperties& boxAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, PositioningType positioning, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& boxColours) {
        RC_ASSERT(Internal::System, "Tried to create a UI slider before initializing UI");
        RC_ASSERT(!Internal::System->Elements.empty(), "Tried to create a UI slider before calling UI Begin");

        Internal::System->Elements.emplace_back(SurfaceType::Activatable, LayoutType::None, positioning, position, size * Internal::System->Elements[Internal::System->OpenElement].Size, boxColours, Internal::System->OpenElement);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::AtlasTextureSliderWidget<T>>(value, min, max, vertical, boxAtlasProperties.Size, boxAtlasProperties.Indices, sliderSize, sliderAtlasProperties.Size, sliderAtlasProperties.Indices);

        Internal::System->Elements[Internal::System->OpenElement].ChildCount++;

        size_t currentIndex = Internal::System->Elements.size() - 1;
        for (size_t i = currentIndex - 1; i > Internal::System->OpenElement; i--) {
            if (Internal::System->Elements[i].ParentID == Internal::System->OpenElement) {
                Internal::System->Elements[i].SiblingID = currentIndex;
                break;
            }
        }
    }
    
    template void UI::TextureSlider<int8_t>(int8_t&, int8_t, int8_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<uint8_t>(uint8_t&, uint8_t, uint8_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<int32_t>(int32_t&, int32_t, int32_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<uint32_t>(uint32_t&, uint32_t, uint32_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<int64_t>(int64_t&, int64_t, int64_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<uint64_t>(uint64_t&, uint64_t, uint64_t, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<float>(float&, float, float, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);
    template void UI::TextureSlider<double>(double&, double, double, bool, const AtlasProperties&, glm::vec2, const AtlasProperties&, PositioningType, glm::vec2, glm::vec2, const std::array<glm::vec4, 3>&);

    void UI::ScrollBar(float& offset, float buttonSize, glm::vec2 position, glm::vec2 size, std::array<glm::vec4, 3>& baseColours, std::array<glm::vec4, 3>& highlightColours) {
        BeginContainer(PositioningType::Relative, position, size, glm::vec4(0.0f), LayoutType::None);
        size_t index = Internal::System->Elements.size() - 1;

        Slider(offset, 0.0f, 0.0f, true, glm::vec2(1.0f, 1.0f - buttonSize * 2), highlightColours, baseColours);

        std::function renderTriangleButton = [highlightColours](Surface& current, float flip) -> bool {
            uint32_t colourIndex = &Internal::System->Elements[Internal::System->ActiveID] == &current ? 2 : &Internal::System->Elements[Internal::System->HoverID] == &current ? 1 : 0;
            glm::vec4& colour = current.Colours[colourIndex];

            Renderer2D::DrawFlatRoundedQuad(current.Size * 0.99f, 0.2f, 2, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), colour);
            Renderer2D::DrawFlatRoundedQuadEdge(current.Size, 0.075f, 0.2f, 5, { current.Position.x, current.Position.y, 0.0f }, glm::vec3(1.0f), { colour.r * 1.2f, colour.g * 1.2f, colour.b * 1.2f, colour.a });

            Renderer2D::DrawFlatTriangle(glm::vec3(60.0f), { current.Position.x, current.Position.y, 0.0f }, { current.Size.x * 0.5f, flip * current.Size.y * 0.5f, 0.0f }, highlightColours[colourIndex]);

            return true;
        };

        bool downButton = Button(PositioningType::Relative, glm::vec2(0.0f, -0.5f * (1.0f - buttonSize)), glm::vec2(1.0f, buttonSize), baseColours[0], baseColours[1], baseColours[2]);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::CustomRenderWidget>(std::bind(renderTriangleButton, std::placeholders::_1, -1.0f));

        bool upButton = Button(PositioningType::Relative, glm::vec2(0.0f, 0.5f * (1.0f - buttonSize)), glm::vec2(1.0f, buttonSize), baseColours[0], baseColours[1], baseColours[2]);
        Internal::System->Elements.back().Widget = std::make_unique<Widgets::CustomRenderWidget>(std::bind(renderTriangleButton, std::placeholders::_1, 1.0f));
        
        Internal::System->Elements[index].Widget = std::make_unique<Widgets::ScrollBarWidget>(offset, downButton, upButton);
        EndContainer();
    }
    
    void UI::TextureScrollBar(float& offset, glm::vec2 buttonOffset, glm::vec2 buttonSize, const AtlasProperties& buttonAtlasProperties, glm::vec2 sliderSize, const AtlasProperties& sliderAtlasProperties, const AtlasProperties& baseAtlasProperties, glm::vec2 position, glm::vec2 size, const std::array<glm::vec4, 3>& colours) {
        BeginContainer(PositioningType::Relative, position, size, colours[0], LayoutType::None);
        size_t index = Internal::System->Elements.size() - 1;

        TextureSlider<float>(offset, 0.0f, 0.0f, true, { glm::uvec3(0), glm::vec2(0.0f) }, sliderSize, sliderAtlasProperties, glm::vec2(1.0f, 1.0f - 0.5f * buttonOffset.y), colours);
        
        bool downButton = TextureButton(buttonAtlasProperties, PositioningType::Relative, glm::vec2(buttonOffset.x, -buttonOffset.y), buttonSize, colours[0], colours[1], colours[2]);
        
        bool upButton = TextureButton(buttonAtlasProperties, PositioningType::Relative, buttonOffset, glm::vec2(buttonSize.x, -buttonSize.y), colours[0], colours[1], colours[2]);
        
        Internal::System->Elements[index].Widget = std::make_unique<Widgets::AtlasTextureScrollBarWidget>(offset, downButton, upButton, baseAtlasProperties.Indices, baseAtlasProperties.Size);
        EndContainer();
    }

    void UI::SetFont(std::shared_ptr<Core::Font> font) { 
        Internal::Font = font; 
    }

    void UI::SetTextureAtlas(std::shared_ptr<Core::Texture2D> atlas, glm::uvec2 atlasSize) {
        Internal::TextureAtlas = atlas;
        Internal::AtlasSize = atlasSize;
    }

    void UI::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<MouseMoved>(OnMouseMovedEvent);
        dispatcher.Dispatch<MouseButtonPressed>(OnMouseButtonPressedEvent);
        dispatcher.Dispatch<MouseButtonReleased>(OnMouseButtonReleasedEvent);
        dispatcher.Dispatch<MouseScrolled>(OnMouseScrollEvent);
        dispatcher.Dispatch<KeyPressed>(OnKeyPressedEvent);
        dispatcher.Dispatch<TextInput>(OnTextInputEvent);
    }

    bool UI::OnMouseMovedEvent(MouseMoved& event){
        Internal::Input->MouseState.Position = event.GetPosition() / Internal::System->Size;

        return false;
    }

    bool UI::OnMouseButtonPressedEvent(MouseButtonPressed& event){
        switch (event.GetButton()) {
        case RC_MOUSE_BUTTON_LEFT:
            Internal::Input->MouseState.Left = Internal::MouseButtonState::Held;
            break;
        case RC_MOUSE_BUTTON_RIGHT:
            Internal::Input->MouseState.Right = Internal::MouseButtonState::Held;
            break;
        }
        if (Internal::System->ActiveID != Internal::System->HoverID && Internal::System->Elements[Internal::System->ActiveID].Type >= SurfaceType::Capture) {
            Internal::System->ActiveID = 0;
        }

        if (!Internal::System->ActiveID || Internal::System->ActiveID == Internal::System->HoverID) {
            if (Internal::System->HoverID >= Internal::System->Elements.size()) {
                return false;
            }

            const Surface& current = Internal::System->Elements[Internal::System->HoverID];
            const Surface& parent = Internal::System->Elements[current.ParentID];

            if (parent.Layout >= LayoutType::Crop && !AABB(Internal::Input->MouseState.Position, parent.Position, parent.Size)) {
                return false;
            }

            if (AABB(Internal::Input->MouseState.Position, current.Position, current.Size)) {
                if (current.Type >= SurfaceType::Activatable) {
                    Internal::System->ActiveID = Internal::System->HoverID;
                }

                return true;
            }
        }

        return false;
    }

    bool UI::OnMouseButtonReleasedEvent(MouseButtonReleased& event){
        switch (event.GetButton()) {
        case RC_MOUSE_BUTTON_LEFT:
            Internal::Input->MouseState.Left = Internal::MouseButtonState::Released;
            break;
        case RC_MOUSE_BUTTON_RIGHT:
            Internal::Input->MouseState.Right = Internal::MouseButtonState::Released;
            break;
        }

        return Internal::System->HoverID || Internal::System->ActiveID;
    }

    bool UI::OnMouseScrollEvent(MouseScrolled& event){
        Internal::Input->MouseState.ScrollOffset = event.GetOffsetY();

        return Internal::System->HoverID || Internal::System->ActiveID;
    }

    bool UI::OnKeyPressedEvent(KeyPressed& event) {
        RC_ASSERT(Internal::Input, "UI should be initialized before dispatching events to it");
        if (Internal::System->Elements[Internal::System->ActiveID].Type != SurfaceType::TextInput) {
            return false;
        }

        switch (event.GetKeyCode()) {
        case RC_KEY_ESCAPE:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Escape] = true;
            return true;
        case RC_KEY_ENTER:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Enter] = true;
            return true;
        case RC_KEY_TAB:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Tab] = true;
            return true;
        case RC_KEY_BACKSPACE:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Backspace] = true;
            return true;
        case RC_KEY_DELETE:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Delete] = true;
            return true;
        case RC_KEY_RIGHT:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Rigth] = true;
            return true;
        case RC_KEY_LEFT:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Left] = true;
            return true;
        case RC_KEY_DOWN:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Down] = true;
            return true;
        case RC_KEY_UP:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Up] = true;
            return true;
        case RC_KEY_PAGE_UP:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::PageUp] = true;
            return true;
        case RC_KEY_PAGE_DOWN:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::PageDown] = true;
            return true;
        case RC_KEY_HOME:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Home] = true;
            return true;
        case RC_KEY_END:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::End] = true;
            return true;
        case RC_KEY_LEFT_SHIFT:
        case RC_KEY_RIGHT_SHIFT:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Shift] = true;
            return true;
        case RC_KEY_LEFT_CONTROL:
        case RC_KEY_RIGHT_CONTROL:
            Internal::Input->KeyboardState.SpecialKeys[Internal::InputKeys::Control] = true;
            return true;
        }

        return false;
    }

    bool UI::OnTextInputEvent(TextInput& event) {
        RC_ASSERT(Internal::Input, "UI should be initialized before dispatching events to it");
        
        if (Internal::System->Elements[Internal::System->ActiveID].Type != SurfaceType::TextInput) {
            return false;
        }

        Internal::Input->KeyboardState.InputedText.push_back(event.GetCharacter());
        return true;
    }
}