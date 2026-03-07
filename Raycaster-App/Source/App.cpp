#include "Core.h"

#include "UILayer.h"
#include "RaycasterLayer.h"
#include "Layer2D.h"
#include "Layer3D.h"
#include "RaycasterScene.h"

extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

class Raycaster : public Core::Application {
public:
	Raycaster() {
		RaycasterScene* scene = new RaycasterScene();
		SetActiveScene(scene);
		
		auto f = new UILayer;
		PushOverlay(f);
		RaycasterLayer* layer0 = new RaycasterLayer();
		PushLayer(layer0);
		Layer3D* layer1 = new Layer3D();
		PushLayer(layer1);
		GetWindow().SetVSync(false);
	}

	~Raycaster(){}
};

Core::Application* Core::CreateApplication() {
	return new Raycaster();
}

int main() {
    Core::Application* app = Core::CreateApplication();
    app->Run();
    delete app;
}
