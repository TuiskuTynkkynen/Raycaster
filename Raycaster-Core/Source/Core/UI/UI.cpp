#include "UI.h"
#include "UI.h"

#include "Core/Base/Application.h"
#include "Core/Base/Input.h"
#include "Core/Debug/Debug.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"

namespace Core::UI {
		struct Surface {
			glm::vec2 Position;
			glm::vec2 Size;

			std::array<glm::vec3, 3> Colours; // [0] = primary, [1] = hover, [2] = clicked
		};

		struct System {
			size_t HoverID = 0;
			size_t ActiveID = 0;

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
		RC_ASSERT(InternalSystem->Elements.empty(), "Begin was called more than once before ending current UI");

		Surface s{ screenPosition, screenSize, {glm::vec4(0.0f), glm::vec4(0.0f), glm::vec4(0.0f)} };
		InternalSystem->Elements.push_back(s);
	}

	void UI::End() {
		Surface& top = InternalSystem->Elements.front();
		RenderAPI::SetViewPort(top.Position.x, top.Position.y, top.Position.x + top.Size.x, top.Position.y + top.Size.y);
		Renderer2D::BeginScene(glm::ortho(0.0f, 1.0f, 1.0f, 0.0f));
		
		//Calculate padding size
		float padding = 1.0f;
		for (size_t i = 1; i < InternalSystem->Elements.size(); i++) {
			padding -= InternalSystem->Elements[i].Size.y;
		}
		padding = std::max(0.05f, padding / (InternalSystem->Elements.size()));
		
		//Calculate hovered and active surface
		float mouseX = Input::GetMouseX() / top.Size.x;
		float mouseY = Input::GetMouseY() / top.Size.y;
		InternalSystem->HoverID = 0;
		if (!Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
			InternalSystem->ActiveID = 0;
		}

		float currentY = padding;
		for (size_t i = 1; i < InternalSystem->Elements.size(); i++) {
			Surface& s = InternalSystem->Elements[i];

			if (InternalSystem->ActiveID && InternalSystem->ActiveID != i) {
				currentY += padding + s.Size.y; 
				continue; 
			}

			if (mouseX <= 0.5f + s.Size.x * 0.5f && mouseX >= 0.5f - s.Size.x * 0.5f
				&& mouseY <= currentY + s.Size.y && mouseY >= currentY) {
				
				InternalSystem->HoverID = i;
				
				if (Input::IsButtonPressed(RC_MOUSE_BUTTON_LEFT)) {
					InternalSystem->ActiveID = i;
				}
			}

			currentY += padding + s.Size.y;
		}

		//Render surfaces
		currentY = padding;
		for (size_t i = 1; i < InternalSystem->Elements.size(); i++) {
			Surface& s = InternalSystem->Elements[i];
			
			uint32_t colourIndex = InternalSystem->ActiveID == i ? 2 : InternalSystem->HoverID == i ? 1 : 0;
			glm::vec3& colour = s.Colours[colourIndex];
			
			if (s.Size.x * s.Size.y == 0.0f) { continue; }

			Renderer2D::DrawFlatQuad({ 0.5f, currentY + s.Size.y * 0.5f, 0.0f }, { s.Size.x, s.Size.y, 0.0f }, colour);
			
			currentY += padding + s.Size.y;
		}

		Renderer2D::EndScene();

		InternalSystem->Elements.clear();
	}

	bool UI::Button(glm::vec2 size, const glm::vec3& primaryColour, const glm::vec3& hoverColour, const glm::vec3& activeColour) {
		Surface s{ glm::vec2(2.0f), size, {primaryColour, hoverColour, activeColour} };
		InternalSystem->Elements.push_back(s);

		return Input::IsButtonReleased(RC_MOUSE_BUTTON_LEFT) && InternalSystem->HoverID == InternalSystem->Elements.size() - 1 && InternalSystem->ActiveID == InternalSystem->Elements.size() - 1;
	}
}