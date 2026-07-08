#include "MenuScene.h"

#include "GameScene.h"
#include "DemoScene.h"

#include "Core/Base/Application.h"
#include "Core/Renderer/RenderAPI.h"
#include "Core/Renderer/Renderer2D.h"
#include "Core/UI/UI.h"

#define INITILIZER_LIST_3(type, ...) { type{__VA_ARGS__}, type{__VA_ARGS__}, type{__VA_ARGS__} }

MenuScene::MenuScene() 
    : m_Background(INITILIZER_LIST_3(Core::Texture2D, Core::Texture2D::WrapMode::ClampToEdge, Core::Texture2D::WrapMode::ClampToEdge, Core::Texture2D::Filter::Linear, Core::Texture2D::Filter::Linear)) 
{
    std::string name = "Assets/Textures/background_?.jpeg";
    for (size_t i = 0; i < m_Background.size(); i++) {
        name[27] = '0' + char(i);
        m_Background[i].BindImage(name.c_str());
    }
}

void MenuScene::OnAttach(Core::Application& app) {
    std::shared_ptr<Core::Font> font = std::make_shared<Core::Font>(false);
    font->AddCharacterRange(' ', '~'); //Printable ASCII
    font->AddCharacterRange(0x00A1, 0x0FF); //Printable Latin-1 Supplement
    font->GenerateAtlas("Assets/Fonts/tiny5/tiny5-Medium.ttf", 8);
    
    Core::UI::Init();
    Core::UI::SetFont(font);

    m_SettingsUI.Init();

    m_ViewPortWidth = Core::Application::GetWindow().GetWidth();
    m_ViewPortHeight = Core::Application::GetWindow().GetHeight();
    m_Framebuffer.Resize(m_ViewPortWidth, m_ViewPortHeight, m_SampleCount);
    m_Framebuffer.InitRender();
}

void MenuScene::OnDetach(Core::Application& app) {
    m_Framebuffer.ShutdownRender();
    m_SettingsUI.Shutdown();
    Core::UI::Shutdown();
}

void MenuScene::OnUpdate(Core::Timestep deltaTime) {
    if (m_ViewPortHeight == 0 || m_ViewPortWidth == 0) {
        return;
    }

    const float aspect = m_ViewPortWidth / float(m_ViewPortHeight);
    
    // Background image
    Core::RenderAPI::SetViewPort(0, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(glm::ortho(-aspect / 2.0f, aspect / 2.0f, -1.0f, 1.0f));
    const float backgroudImageWidth = glm::max(aspect, 6.0f) / m_Background.size();
    for (uint32_t i = 0; i < m_Background.size(); i++) {
        m_Background[i].Activate(8 + i);
        Core::Renderer2D::DrawQuad(8 + i, glm::vec4(1), glm::vec3(i * backgroudImageWidth - backgroudImageWidth, 0, 0), glm::vec3(backgroudImageWidth, 2, 0), glm::vec2(0), glm::vec2(1));
    }
    Core::Renderer2D::EndScene();

    // UI
    m_Framebuffer.Activate();
    m_Framebuffer.Clear();

    const uint32_t maxWidth = glm::min(m_ViewPortWidth, m_ViewPortHeight * 4);
    const uint32_t offset = (int32_t)(m_ViewPortWidth - maxWidth) / 2;
    Core::UI::Begin({ offset, 0 }, { maxWidth, m_ViewPortHeight }, Core::UI::LayoutType::Vertical, glm::vec4(0.0f));
    Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { 0.0f, -0.025f, 0.0f }, { 1.0f, 1.0f }, glm::vec4(0.0f));

    if (m_SettingsUI.IsEnabled) {
        m_SettingsUI.Render();
    } else {
        MainMenu();
    }

    Core::UI::EndContainer();
    Core::UI::End(deltaTime);

    m_Framebuffer.Deactivate();
    
    Core::RenderAPI::SetViewPort(0, 0, m_ViewPortWidth, m_ViewPortHeight);
    m_Framebuffer.Render();
}

void MenuScene::MainMenu() {
    if (Core::UI::Button("Game", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushScene<GameScene>();
    }

    if (Core::UI::Button("Demo", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushScene<DemoScene>();
    }

    if (Core::UI::Button("Settings", glm::vec2{ 0.5f, 0.2f })) {
        Core::UI::ResetInteractionElement();
        m_SettingsUI.IsEnabled = true;
    }

    if (Core::UI::Button("Exit", glm::vec2{ 0.5f, 0.2f })) {
        Core::UI::ResetInteractionElement();
        Core::Application::PopScene();
    }
}

void MenuScene::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>([this](auto& e) { return OnWindowResizeEvent(e); });

    m_SettingsUI.OnEvent(event);
    Core::UI::OnEvent(event);
}

bool MenuScene::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth();
    m_ViewPortHeight = event.GetHeight();

    if (m_ViewPortHeight != 0 && m_ViewPortWidth != 0) {
        m_Framebuffer.Resize(m_ViewPortWidth, m_ViewPortHeight, m_SampleCount);
    }

    return false;
}
