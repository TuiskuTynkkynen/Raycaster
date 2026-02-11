#include "UILayer.h"

#include "Core/UI/UI.h"

void UILayer::OnAttach() {
    m_ViewPortWidth = Core::Application::GetWindow().GetWidth();
    m_ViewPortHeight = Core::Application::GetWindow().GetHeight();
    
    Core::Renderer2D::SetTextureAtlas("Assets/Textures/wolfenstein_texture_atlas.png", ATLASWIDTH, ATLASHEIGHT);

    std::shared_ptr<Core::Font> font = std::make_shared<Core::Font>(false);
    font->AddCharacterRange(' ', '~'); //Printable ASCII
    font->AddCharacterRange(0x00A1, 0x0FF); //Printable Latin-1 Supplement
    font->GenerateAtlas("Assets/Fonts/tiny5/tiny5-Medium.ttf", 8);
    Core::Renderer2D::SetFont(font);

    Core::UI::Init();
    Core::UI::SetFont(font);
    std::shared_ptr<Core::Texture2D> buttonTexture = std::make_unique<Core::Texture2D>(Core::Texture2D::WrapMode::Repeat, Core::Texture2D::WrapMode::Repeat, Core::Texture2D::Filter::Nearest, Core::Texture2D::Filter::Nearest);
    buttonTexture->BindImage("Assets/Textures/Button.png");
    Core::UI::SetTextureAtlas(buttonTexture, glm::uvec2(12, 7));

    m_SettingsUI.Init();
}

void UILayer::OnDetach() {
    m_SettingsUI.Shutdown();
    Core::UI::Shutdown();
}

void UILayer::OnUpdate(Core::Timestep deltaTime) {
    RC_ASSERT(dynamic_cast<RaycasterScene*>(m_Scene.get()));
    const RaycasterScene& scene = static_cast<RaycasterScene&>(*m_Scene);

    Core::UI::Begin({ 0.0f, 0.0f }, { m_ViewPortWidth, m_ViewPortHeight }, Core::UI::LayoutType::Horizontal, glm::vec4(0.0f));
    static float timeDelta = 0;
    static uint32_t frameCount = 0;
    static float frameTime = 0;

    frameCount++;
    timeDelta += deltaTime;
    if (timeDelta >= 0.1f) {
        frameTime = 1000.0f * timeDelta / frameCount;

        frameCount = 0;
        timeDelta = 0.0f;
    }

    std::string frameStats = std::to_string(int(1000 / frameTime)) + " FPS\n" + std::to_string(frameTime) + " ms";
    Core::UI::Text(frameStats, 0.5f, Core::UI::TextAlignment::Left, Core::UI::PositioningType::Relative, { -0.495f, -0.485f }, { 0.0f, 0.075f }, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    std::string playerStats = "Health: " + std::format("{:2}", scene.GetPlayer().GetHealth()) + "/" + std::to_string(static_cast<uint32_t>(Player::MaxHealth));
    Core::UI::Text(playerStats, 0.5f, Core::UI::TextAlignment::Right, Core::UI::PositioningType::Relative, { 0.495f, -0.485f }, { 0.0f, 0.075f }, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    RaycasterScene::State sceneState = scene.GetState();
    if (sceneState != RaycasterScene::State::Running) {
    Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f }, { 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f, 0.5f });
        if (m_SettingsUI.IsEnabled) {
            m_SettingsUI.Render();
        } else {
            PauseScreen(scene);
        }
    Core::UI::EndContainer();
    }

    Core::UI::End(deltaTime);
}

void UILayer::PauseScreen(const RaycasterScene& scene) {
    RaycasterScene::State sceneState = scene.GetState();

    Core::UI::Text(sceneState == RaycasterScene::State::Dead ? "You died!" : "Paused", glm::vec2{ 0.5f, 0.25f }, glm::vec4(1.0f));

    if (sceneState >= RaycasterScene::State::Dead && Core::UI::Button("Restart", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Restart>();
    }

    if (sceneState == RaycasterScene::State::Paused && Core::UI::Button("Continue", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Resume>();
    }

    if (Core::UI::Button("Settings", glm::vec2{ 0.5f, 0.2f })) {
        m_SettingsUI.IsEnabled = true;
    }

    if (Core::UI::Button("Quit", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Core::ApplicationClose>();
    }
}

void UILayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&UILayer::OnWindowResizeEvent, this, std::placeholders::_1));

    m_SettingsUI.OnEvent(event);
    Core::UI::OnEvent(event);
}

bool UILayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth();
    m_ViewPortHeight = event.GetHeight();

    return false;
}