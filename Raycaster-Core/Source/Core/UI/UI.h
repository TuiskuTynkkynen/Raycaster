#pragma once

#include <glm/glm.hpp>

#include <memory>
#include <array>
#include <vector>



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

namespace Core::UI {
	namespace {
		std::unique_ptr<System> InternalSystem;
		std::array<glm::vec3, 3> DefaultColours = {
			glm::vec3(0.125f, 0.125f, 0.5f),
			glm::vec3(0.325f, 0.325f, 0.7f),
			glm::vec3(0.7f, 0.7f, 0.325f)
		};
	}

	void Init();
	void Shutdown();

	void Begin(glm::uvec2 screenPosition, glm::uvec2 screenSize);
	void End();

	bool Button(glm::vec2 relativeSize, const glm::vec3& primaryColour = DefaultColours[0], const glm::vec3& hoverColour = DefaultColours[1], const glm::vec3& activeColour = DefaultColours[2]);
}
