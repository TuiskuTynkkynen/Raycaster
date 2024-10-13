#include "UI.h"
#include "UI.h"

#include "Core/Base/Input.h"
#include "Core/Debug/Debug.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

namespace Core::UI {
	enum class SurfaceType {
		None = 0,
		Button,
	};
		
	struct Surface {
		SurfaceType Type = SurfaceType::None;
		
		glm::vec2 Position;
		glm::vec2 Size;

		std::array<glm::vec4, 3> Colours; // [0] = primary, [1] = hover, [2] = clicked
		
		size_t ParentID = 0;
		size_t SiblingID = 0;
		uint32_t ChildCount = 0;
	};

	struct System {
		glm::vec2 Position;
		glm::vec2 Size;

		size_t HoverID = 0;
		size_t ActiveID = 0;

		size_t OpenElement = 0;

		std::vector<Surface> Elements;
	};

	std::unique_ptr<System> InternalSystem;
}

namespace Core {
	void UI::Init() {
		RC_ASSERT(!InternalSystem, "UI has already been initialized");

		InternalSystem = std::make_unique<System>();
	}
	
	void UI::Shutdown() {
		RC_ASSERT(InternalSystem, "UI has not been initialized");

		InternalSystem.reset();
	}

	void UI::Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize) {
		RC_ASSERT(InternalSystem, "UI has not been initialized");
		InternalSystem->Elements.clear();

		InternalSystem->Position = screenPosition;
		InternalSystem->Size = screenSize;

		Surface s{ SurfaceType::None, glm::vec2(0.5f), glm::vec2(1.0f), {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)}};
		InternalSystem->Elements.emplace_back(s);
	}

	void UI::Update() {
		RC_ASSERT(InternalSystem, "Tried to update UI before initializing");
		RC_ASSERT(!InternalSystem->Elements.empty(), "Tried to update UI before calling UI Begin");

		auto CalculatePadding = [](const Surface& s) {
			const Surface& parent = InternalSystem->Elements[s.ParentID];

			float padding = s.ParentID ? parent.Size.y : 1.0f;

			for (size_t i = s.ParentID + 1; i && i < InternalSystem->Elements.size(); i = InternalSystem->Elements[i].SiblingID) {
				padding -= InternalSystem->Elements[i].Size.y;
			}

			return std::max(0.05f * parent.Size.y, padding / (parent.ChildCount + 1));
		};

		float mouseX = Input::GetMouseX() / InternalSystem->Size.x;
		float mouseY = Input::GetMouseY() / InternalSystem->Size.y;
		InternalSystem->HoverID = 0;
		if (!Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
			InternalSystem->ActiveID = 0;
		}

		size_t lastParentId = -1;
		float padding = 0.0f;
		float relativeY = 0.0f;
		for (size_t i = 1; i < InternalSystem->Elements.size(); i++) {
			Surface& current = InternalSystem->Elements[i];
			const Surface& parent = InternalSystem->Elements[current.ParentID];

			if (current.ParentID != lastParentId) { 
				padding = CalculatePadding(current);
				relativeY = padding;
			}

			current.Position.x = parent.Position.x;
			current.Position.y = parent.Position.y - parent.Size.y * 0.5f + relativeY + current.Size.y * 0.5f;

			if (current.Type == SurfaceType::Button && (!InternalSystem->ActiveID || InternalSystem->ActiveID == i)) {
				if (mouseX <= current.Position.x + current.Size.x * 0.5f && mouseX >= current.Position.x - current.Size.x * 0.5f
					&& mouseY <= current.Position.y + current.Size.y * 0.5f && mouseY >= current.Position.y - current.Size.y * 0.5f) {

					InternalSystem->HoverID = i;

					if (Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
						InternalSystem->ActiveID = i;
					}
				}
			}

			relativeY += current.Size.y + padding;
			lastParentId = current.ParentID;
		}
	}

	void UI::Render() {
		RC_ASSERT(InternalSystem, "Tried to render UI before initializing");
		RC_ASSERT(!InternalSystem->Elements.empty(), "Tried to render UI before calling UI Begin");

		RenderAPI::SetViewPort(InternalSystem->Position.x, InternalSystem->Position.y, InternalSystem->Position.x + InternalSystem->Size.x, InternalSystem->Position.y + InternalSystem->Size.y);
		Renderer2D::BeginScene(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f));
		
		for (size_t i = 1; i < InternalSystem->Elements.size(); i++) {
			Surface& s = InternalSystem->Elements[i];

			uint32_t colourIndex = InternalSystem->ActiveID == i ? 2 : InternalSystem->HoverID == i ? 1 : 0;
			glm::vec4& colour = s.Colours[colourIndex];

			if (s.Size.x * s.Size.y == 0.0f) { continue; }

			Renderer2D::DrawFlatQuad({ s.Position.x, s.Position.y, 0.0f }, { s.Size.x, s.Size.y, 0.0f }, colour);
		}

		Renderer2D::EndScene();
	}

	void UI::End() {
		Update();
		Render();

		InternalSystem->Elements.clear();
	}

	void UI::BeginContainer(glm::vec2 size, const glm::vec4& primaryColour) {
		RC_ASSERT(InternalSystem, "Tried to begin a UI container before initializing UI");
		RC_ASSERT(!InternalSystem->Elements.empty(), "Tried to begin a UI container before calling UI Begin");

		Surface s{ SurfaceType::None, glm::vec2(2.0f), size, {primaryColour}, InternalSystem->OpenElement};
		InternalSystem->Elements.push_back(s);

		InternalSystem->Elements[InternalSystem->OpenElement].ChildCount++;

		size_t currentIndex = InternalSystem->Elements.size() - 1;
		for (size_t i = currentIndex - 1; i > InternalSystem->OpenElement; i--) {
			if (InternalSystem->Elements[i].ParentID == InternalSystem->OpenElement) {
				InternalSystem->Elements[i].SiblingID = currentIndex;
				break;
			}
		}

		InternalSystem->OpenElement = currentIndex;
	}

	void UI::EndContainer() {
		size_t parentId = InternalSystem->Elements[InternalSystem->OpenElement].ParentID;
		RC_ASSERT(parentId != -1, "Tried to end a UI container before begining one");
		
		InternalSystem->OpenElement = parentId;
	}

	bool UI::Button(glm::vec2 size, const glm::vec4& primaryColour, const glm::vec4& hoverColour, const glm::vec4& activeColour) {
		RC_ASSERT(InternalSystem, "Tried to create a UI button before initializing UI");
		RC_ASSERT(!InternalSystem->Elements.empty(), "Tried to create a UI button before calling UI Begin");

		Surface s{ SurfaceType::Button, glm::vec2(2.0f), size * InternalSystem->Elements[InternalSystem->OpenElement].Size, {primaryColour, hoverColour, activeColour}, InternalSystem->OpenElement };
		InternalSystem->Elements.push_back(s);
		
		InternalSystem->Elements[InternalSystem->OpenElement].ChildCount++;

		size_t currentIndex = InternalSystem->Elements.size() - 1;
		for (size_t i = currentIndex - 1; i > InternalSystem->OpenElement; i--) {
			if (InternalSystem->Elements[i].ParentID == InternalSystem->OpenElement) {
				InternalSystem->Elements[i].SiblingID = currentIndex;
				break;
			}
		}

		return Input::IsButtonReleased(RC_MOUSE_BUTTON_LEFT) && InternalSystem->HoverID == currentIndex && InternalSystem->ActiveID == currentIndex;
	}
}