#include "Core.h"

int main(){
    Core::Application* app = Core::CreateApplication();
    app->Run();
    delete app;
}
