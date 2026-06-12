#include "Core.h"

#include "RaycasterScene.h"

#ifdef PLATFORM_WINDOWS
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

class Raycaster : public Core::Application {
public:
    Raycaster() {
        SetActiveScene(new RaycasterScene);
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
