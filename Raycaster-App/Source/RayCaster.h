#pragma once

#include "Core.h"

class RayCaster : public Core::Application
{
public:
	RayCaster();
	~RayCaster();

	void Run() override;
};

Core::Application* Core::CreateApplication() {
	return new RayCaster();
}