#include "MenuScene.h"

#include "Core/Base/Application.h"

#ifdef PLATFORM_WINDOWS
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int main() {
    Core::Application app({
        .IconPaths = {
            "Assets/Textures/icon.png",
            "Assets/Textures/icon_medium.png",
            "Assets/Textures/icon_large.png",
        }
    });

    app.PushScene<MenuScene>();
    app.GetWindow().SetVSync(false);

    app.Run();
}
