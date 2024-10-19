#include "UI.h"
#include "UI.h"

#include "Layout.h"
#include "Internal.h"

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

		Surface s{ SurfaceType::None, layout, glm::vec2(0.5f), glm::vec2(1.0f), {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)}};
		UI::Internal::System->Elements.emplace_back(s);
	}

	void UI::Update() {
		RC_ASSERT(UI::Internal::System, "Tried to update UI before initializing");
		RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to update UI before calling UI Begin");

		auto CalculatePadding = [](const Surface& s) {
			const Surface& parent = UI::Internal::System->Elements[s.ParentID];

			float padding = s.ParentID ? parent.Size.y : 1.0f;

			for (size_t i = s.ParentID + 1; i && i < UI::Internal::System->Elements.size(); i = UI::Internal::System->Elements[i].SiblingID) {
				padding -= UI::Internal::System->Elements[i].Size.y;
			}

			return std::max(0.05f * parent.Size.y, padding / (parent.ChildCount + 1));
		};

		float mouseX = Input::GetMouseX() / UI::Internal::System->Size.x;
		float mouseY = Input::GetMouseY() / UI::Internal::System->Size.y;
		UI::Internal::System->HoverID = 0;
		if (!Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
			UI::Internal::System->ActiveID = 0;
		}

		size_t lastParentId = -1;
		float padding = 0.0f;
		float relativeY = 0.0f;
		for (size_t i = 1; i < UI::Internal::System->Elements.size(); i++) {
			Surface& current = UI::Internal::System->Elements[i];
			const Surface& parent = UI::Internal::System->Elements[current.ParentID];

			if (current.ParentID != lastParentId) {
				padding = CalculatePadding(current);
				relativeY = padding;
			}

			current.Position.x = parent.Position.x;
			current.Position.y = parent.Position.y - parent.Size.y * 0.5f + relativeY + current.Size.y * 0.5f;

			if (current.Type == SurfaceType::Button && (!UI::Internal::System->ActiveID || UI::Internal::System->ActiveID == i)) {
				if (mouseX <= current.Position.x + current.Size.x * 0.5f && mouseX >= current.Position.x - current.Size.x * 0.5f
					&& mouseY <= current.Position.y + current.Size.y * 0.5f && mouseY >= current.Position.y - current.Size.y * 0.5f) {

					UI::Internal::System->HoverID = i;

					if (Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
						UI::Internal::System->ActiveID = i;
					}
				}
			}

			relativeY += current.Size.y + padding;
			lastParentId = current.ParentID;
		}
	}

	void UI::Render() {
		RC_ASSERT(UI::Internal::System, "Tried to render UI before initializing");
		RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to render UI before calling UI Begin");

		RenderAPI::SetViewPort(UI::Internal::System->Position.x, UI::Internal::System->Position.y, UI::Internal::System->Position.x + UI::Internal::System->Size.x, UI::Internal::System->Position.y + UI::Internal::System->Size.y);
		Renderer2D::BeginScene(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f));
		
		for (size_t i = 1; i < UI::Internal::System->Elements.size(); i++) {
			Surface& s = UI::Internal::System->Elements[i];

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

	void UI::BeginContainer(glm::vec2 size, const glm::vec4& primaryColour, LayoutType layout) {
		RC_ASSERT(UI::Internal::System, "Tried to begin a UI container before initializing UI");
		RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to begin a UI container before calling UI Begin");

		Surface s{ SurfaceType::None, layout, glm::vec2(2.0f), size * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, {primaryColour}, UI::Internal::System->OpenElement};
		UI::Internal::System->Elements.push_back(s);

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

	bool UI::Button(glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
		RC_ASSERT(UI::Internal::System, "Tried to create a UI button before initializing UI");
		RC_ASSERT(!UI::Internal::System->Elements.empty(), "Tried to create a UI button before calling UI Begin");

		Surface s{ SurfaceType::Button, LayoutType::None, glm::vec2(2.0f), size * UI::Internal::System->Elements[UI::Internal::System->OpenElement].Size, {primaryColour, hoverColour, activeColour}, UI::Internal::System->OpenElement };
		UI::Internal::System->Elements.push_back(s);
		
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
}