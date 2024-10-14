#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <array>
#include <vector>

namespace Core::UI {
	enum class LayoutType {
		None = 0,
		Vertical,
		Horizontal,
	};

	namespace{
		std::array<glm::vec4, 3> DefaultColours = {
			glm::vec4(0.125f, 0.125f, 0.5f, 1.0f),
			glm::vec4(0.325f, 0.325f, 0.7f, 1.0f),
			glm::vec4(0.7f, 0.7f, 0.325f, 1.0f)
		};
	}

	void Init();
	void Shutdown();

	void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize, LayoutType layout = LayoutType::Vertical);
	void Update();
	void Render();
	void End();

	void BeginContainer(glm::vec2 relativeSize, const glm::vec4& colour = DefaultColours[0], LayoutType layout = LayoutType::Vertical);
	void EndContainer();

	bool Button(glm::vec2 relativeSize, const glm::vec4& primaryColour = DefaultColours[0], const glm::vec4& hoverColour = DefaultColours[1], const glm::vec4& activeColour = DefaultColours[2]);
}

