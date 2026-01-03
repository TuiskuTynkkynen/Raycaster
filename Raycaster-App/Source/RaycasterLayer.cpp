#include "RaycasterLayer.h"

#include "RaycasterScene.h"

#include "Core/UI/UI.h"
#include "Core/Audio/Audio.h"
#include "Core/Audio/Bus.h"

void RaycasterLayer::OnAttach() {
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

    Core::Audio::Init();

    Core::Audio::SetMasterPitch(0.5f);
    Core::Audio::SetMasterBalance(0.5f);

    Core::Audio::Sound::Flags flags(Core::Audio::Sound::DisablePitch, Core::Audio::Sound::DisableSpatialization);
    Core::Audio::GetSoundManager().RegisterSound("sound", "Assets/Audio/test.wav", flags);

    auto& master = Core::Audio::GetMasterBus();
    master.AddFilter(Core::Audio::Effects::DelaySettings(std::chrono::milliseconds(100), 0.5f));
    master.AddFilter(Core::Audio::Effects::LowPassSettings(1000.0));
}

void RaycasterLayer::OnDetach() {
    Core::Audio::Shutdown();
}

void RaycasterLayer::OnUpdate(Core::Timestep deltaTime) { 
    static glm::mat4 identity(1.0f);

    Core::RenderAPI::SetViewPort(0, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(identity);

    glm::vec4 colour, colour1;
    glm::vec3 rayPos(0.0f);
    glm::vec3 rayScale(0.0f, RaycastRenderer::GetRayWidth(), 0.0f);
    glm::vec2 texScale(0.0f);

    const auto& floors = static_cast<RaycasterScene&>(*m_Scene).GetFloors();
    float rot = -static_cast<RaycasterScene&>(*m_Scene).GetPlayer().GetRotation() + 90.0f;

    for (const auto& ray : floors) {
        rayPos.x = ray.Position.x;
        rayPos.y = ray.Position.y;

        rayScale.x = ray.Length;
        texScale.x = ray.Length * (m_ViewPortWidth / (2.0f * m_ViewPortHeight * ray.Position.y));

        colour = glm::vec4(ray.BrightnessEnd);
        colour.a = 1.0f;

        colour1 = glm::vec4(ray.BrightnessStart);
        colour1.a = 1.0f;

        Core::Renderer2D::DrawTextureGradientQuad(rayPos, rayScale, colour, colour1, ray.TexturePosition, texScale, ray.TopAtlasIndex, rot);

        rayPos.y *= -1.0f;
        Core::Renderer2D::DrawTextureGradientQuad(rayPos, rayScale, colour, colour1, ray.TexturePosition, texScale, ray.BottomAtlasIndex, rot);
    }

    rayScale.x = RaycastRenderer::GetRayWidth();
    texScale = glm::vec2(0.0f, 1.0f);

    std::span<const Ray> rays = static_cast<RaycasterScene&>(*m_Scene).GetRays();
    for (const auto& ray : rays) {
        rayPos.x = ray.Position.x;
        rayPos.y = ray.Position.y;
        rayScale.y = ray.Scale;

        colour = glm::vec4(ray.Brightness);
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, ray.TexPosition, texScale, ray.Atlasindex);
    }

    Core::Renderer2D::EndScene();
    
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
    
    std::wstring frameStats = std::to_wstring(int(1000/ frameTime)) + L" FPS\n" + std::to_wstring(frameTime) + L" ms";
    Core::UI::Text(frameStats, 0.5f, Core::UI::PositioningType::Relative, {-0.495f, -0.47f}, {0.125f, 0.075f}, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    float health = static_cast<RaycasterScene&>(*m_Scene).GetPlayer().GetHealth();
    std::string playerStats = "Health: " + std::format("{:2}", health) + "/" + std::to_string(static_cast<uint32_t>(Player::MaxHealth));
    Core::UI::Text(playerStats, 0.5f, Core::UI::PositioningType::Relative, {0.3f, -0.47f}, {0.125f, 0.075f}, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    RaycasterScene::State sceneState = static_cast<RaycasterScene&>(*m_Scene).GetState();
    if (sceneState != RaycasterScene::State::Running) {
        Core::UI::BeginContainer(Core::UI::PositioningType::Offset, {-0.025f, 0.0f}, { 1.0f, 1.0f }, { 0.1f, 0.1f, 0.1f, 0.5f });
            Core::UI::Text(sceneState == RaycasterScene::State::Dead ? "You died!" : "Paused", { 0.5f, 0.2f }, glm::vec4(1.0f));
            
            if (sceneState >= RaycasterScene::State::Dead && Core::UI::Button("Restart", { 0.5f, 0.2f })) {
                Core::Application::PushEvent<Restart>();
            }
            
            if (sceneState == RaycasterScene::State::Paused && Core::UI::Button("Continue", { 0.5f, 0.2f })) {
                Core::Application::PushEvent<Resume>();
            }
            
            if (Core::UI::Button("Quit", { 0.5f, 0.2f })) {
                Core::Application::PushEvent<Core::ApplicationClose>();
            }
        Core::UI::EndContainer();
    }

    Core::UI::End(deltaTime);
}

void RaycasterLayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&RaycasterLayer::OnWindowResizeEvent, this, std::placeholders::_1));

    Core::UI::OnEvent(event);
}

bool RaycasterLayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() / 2;
    m_ViewPortHeight = event.GetHeight();

    return false;
}