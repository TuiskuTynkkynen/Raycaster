#pragma once

#include "Core.h"

class Raycaster : public Core::Application
{
public:
	Raycaster();
	~Raycaster();
	
	void Run() override;
};

Core::Application* Core::CreateApplication() {
	return new Raycaster();
}