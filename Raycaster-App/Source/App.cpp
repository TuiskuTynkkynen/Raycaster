#include "Core.h"

#include "RaycasterLayer.h"
#include "RaycasterScene.h"

class Raycaster : public Core::Application {
public:
	Raycaster() {
		RaycasterScene* scene = new RaycasterScene();
		SetActiveScene(scene);
		
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
