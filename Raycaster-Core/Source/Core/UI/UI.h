#pragma once

#include "Types.h"

namespace Core::UI {
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

