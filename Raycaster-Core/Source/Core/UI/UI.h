#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <array>
#include <vector>

namespace Core::UI {
	namespace{
		std::array<glm::vec3, 3> DefaultColours = {
			glm::vec3(0.125f, 0.125f, 0.5f),
			glm::vec3(0.325f, 0.325f, 0.7f),
			glm::vec3(0.7f, 0.7f, 0.325f)
		};
	}

	void Init();
	void Shutdown();

	void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize);
	void Update();
	void Render();
	void End();

	void BeginContainer(glm::vec2 relativeSize, const glm::vec3& colour = DefaultColours[0]);
	void EndContainer();

	bool Button(glm::vec2 relativeSize, const glm::vec3& primaryColour = DefaultColours[0], const glm::vec3& hoverColour = DefaultColours[1], const glm::vec3& activeColour = DefaultColours[2]);
}

