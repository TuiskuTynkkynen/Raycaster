#include "RaycasterScene.h"

#include "Core/Base/Application.h"

#ifdef PLATFORM_WINDOWS
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main() {
    Core::Application app;

    app.PushScene<RaycasterScene>();
    app.GetWindow().SetVSync(false);

    app.Run();
}
