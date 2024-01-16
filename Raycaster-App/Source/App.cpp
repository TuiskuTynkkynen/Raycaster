#include "Core.h"

#include "RaycasterLayer.h"

#include <iostream>

class Raycaster : public Core::Application
{
public:
	Raycaster(){}
	~Raycaster(){}
};

Core::Application* Core::CreateApplication() {
	return new Raycaster();
}

int main(){
	Core::Layer* layer = new RaycasterLayer();

    Core::Application* app = Core::CreateApplication();
	app->PushLayer(layer);
    app->Run();
    delete app;
}
