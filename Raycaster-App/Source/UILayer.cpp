#include "UILayer.h"

#include "Keybinds.h"

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
}

void UILayer::OnDetach() {
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
    Core::UI::Text(frameStats, 0.5f, Core::UI::TextAlignment::Left, Core::UI::PositioningType::Relative, { -0.495f, -0.47f }, { 0.0f, 0.075f }, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    std::string playerStats = "Health: " + std::format("{:2}", scene.GetPlayer().GetHealth()) + "/" + std::to_string(static_cast<uint32_t>(Player::MaxHealth));
    Core::UI::Text(playerStats, 0.5f, Core::UI::TextAlignment::Right, Core::UI::PositioningType::Relative, { 0.495f, -0.47f }, { 0.0f, 0.075f }, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    RaycasterScene::State sceneState = scene.GetState();
    if (sceneState != RaycasterScene::State::Running) {
        if (m_ShowKeyBinds) {
            KeyBindsScreen(scene);
        } else {
            PauseScreen(scene);
        }
    }

    Core::UI::End(deltaTime);
}

void UILayer::PauseScreen(const RaycasterScene& scene) {
    RaycasterScene::State sceneState = scene.GetState();

    Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f }, { 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f, 0.5f });
    Core::UI::Text(sceneState == RaycasterScene::State::Dead ? "You died!" : "Paused", glm::vec2{ 0.5f, 0.25f }, glm::vec4(1.0f));

    if (sceneState >= RaycasterScene::State::Dead && Core::UI::Button("Restart", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Restart>();
    }

    if (sceneState == RaycasterScene::State::Paused && Core::UI::Button("Continue", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Resume>();
    }

    if (Core::UI::Button("Key Binds", glm::vec2{ 0.5f, 0.2f })) {
        m_ShowKeyBinds = true;
    }

    if (Core::UI::Button("Quit", glm::vec2{ 0.5f, 0.2f })) {
        Core::Application::PushEvent<Core::ApplicationClose>();
    }
    Core::UI::EndContainer();
}

void UILayer::KeyBindsScreen(const RaycasterScene& scene) {
    static std::array<glm::vec4, 3> deselectedColours = { Core::UI::DefaultColours[0], Core::UI::DefaultColours[0], Core::UI::DefaultColours[0] };
    static std::array<glm::vec4, 3> selectedColours = { Core::UI::DefaultColours[2], Core::UI::DefaultColours[2], Core::UI::DefaultColours[2] };
    bool allDefault = true;

Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f }, { 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f, 0.5f });

    Core::UI::Text("Key Binds", { 0.5f, 0.125f }, glm::vec4(1.0f));

    static float scrollOffset = 0;
    Core::UI::BeginScrollContainer(scrollOffset, { 0.75f, 0.65f }, true, 1.0f, glm::vec4(0.0f), glm::vec4(0.0f));
    for (uint32_t i = 0; i < s_KeyBinds.size(); i++) {
        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, { -0.025f, 0.0f }, { 0.925f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
        Core::UI::Text(s_KeyBinds[i].GetName(), { 0.5f, 1.0f });

        bool isSelected = m_SelectedKeyBind == i;
        if (Core::UI::Button(Core::Input::KeyCodeToString(s_KeyBinds[i].KeyCode), { 0.2f, 1.0f }, isSelected ? selectedColours : Core::UI::DefaultColours)) {
            m_SelectedKeyBind = isSelected ? -1 : i;
        }

        bool isDefault = s_KeyBinds[i].KeyCode == s_KeyBinds[i].GetDefaultKeyCode();
        allDefault &= isDefault;

        if (Core::UI::Button("Reset", { 0.2f, 1.0f }, isDefault ? deselectedColours : Core::UI::DefaultColours, isDefault ? selectedColours : Core::UI::DefaultTextColours)) {
            s_KeyBinds[i].Reset();
        }
        Core::UI::EndContainer();
    }
        Core::UI::ScrollBar(scrollOffset, { 0.05f, 1.0f });
    Core::UI::EndScrollContainer();

    Core::UI::BeginContainer({ 0.5f, 0.125f }, glm::vec4(0.0f), Core::UI::LayoutType::Horizontal);
        if (Core::UI::Button("Reset to Defaults", { 0.5f, 1.0f }, allDefault ? deselectedColours : Core::UI::DefaultColours, allDefault ? selectedColours : Core::UI::DefaultTextColours)) {
            for (KeyBind& bind : s_KeyBinds) {
                bind.Reset();
            }
        }

        if (Core::UI::Button("Back", { 0.5f, 1.0f })) {
            m_ShowKeyBinds = false;
        }
    Core::UI::EndContainer();

Core::UI::EndContainer();
}

void UILayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&UILayer::OnWindowResizeEvent, this, std::placeholders::_1));
    dispatcer.Dispatch<Core::KeyReleased>(std::bind(&UILayer::OnKeyReleased, this, std::placeholders::_1));

    Core::UI::OnEvent(event);
}

bool UILayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth();
    m_ViewPortHeight = event.GetHeight();

    return false;
}

bool UILayer::OnKeyReleased(Core::KeyReleased& event) {
    if (!m_ShowKeyBinds) {
        return false;
    }

    if (event.GetKeyCode() == RC_KEY_ESCAPE) {
        m_ShowKeyBinds = m_SelectedKeyBind != -1;
        m_SelectedKeyBind = -1;
        return true;
    }

    if (m_SelectedKeyBind != -1 && m_SelectedKeyBind < s_KeyBinds.size()) {
        s_KeyBinds[m_SelectedKeyBind].KeyCode = event.GetKeyCode();
        m_SelectedKeyBind = -1;
        return true;
    }
    
    return false;
}