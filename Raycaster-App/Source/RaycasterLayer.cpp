#include "RaycasterLayer.h"

#include "Core/UI/UI.h"
#include "Core/Audio/Audio.h"
#include "Core/Audio/Bus.h"

#include <glm/gtc/matrix_transform.hpp>

void RaycasterLayer::OnAttach() {
    Core::Renderer2D::SetTextureAtlas("wolfenstein_texture_atlas.png", 11, 2);

    std::shared_ptr<Core::Font> font = std::make_shared<Core::Font>(false);
    font->AddCharacterRange(' ', '~'); //Printable ASCII
    font->AddCharacterRange(0x00A1, 0x0FF); //Printable Latin-1 Supplement
    font->GenerateAtlas("tiny5/tiny5-Medium.ttf", 8);
    Core::Renderer2D::SetFont(font);
    
    Core::UI::Init();
    Core::UI::SetFont(font);
    std::shared_ptr<Core::Texture2D> buttonTexture = std::make_unique<Core::Texture2D>(GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST);
    buttonTexture->BindImage("Button.png");
    Core::UI::SetTextureAtlas(buttonTexture, glm::uvec2(12, 7));

    Core::Audio::Init();

    Core::Audio::SetMasterPitch(0.5f);
    Core::Audio::SetMasterBalance(0.5f);

    Core::Audio::Sound::Flags flags(Core::Audio::Sound::DisablePitch, Core::Audio::Sound::DisableSpatialization);
    Core::Audio::GetSoundManager().RegisterSound("sound", "test.wav", flags);

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
    
    const std::vector<Core::Ray>& rays = m_Scene->GetRays();
    uint32_t rayCount = m_Scene->GetRayCount();
    uint32_t rayArraySize = rays.size();
    glm::vec4 colour;
    
    glm::vec3 rayPos(0.0f);
    glm::vec3 rayScale(2.0f, 2.0f / rayCount, 0.0f);
    glm::vec2 texScale(0.0f, 0.0f);

    for (int i = rayCount; i < rayCount * 2; i++) {
        rayPos.y = rays[i].Position.y;
        texScale.x = rays[i].Scale;

        colour = glm::vec4(1.0f) * rays[i].Brightness;
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex, rays[i].TexRotation);
    }

    rayPos.y = 0.0f;
    rayScale.x = 2.0f / rayCount;
    texScale = glm::vec2(0.0f, 1.0f);

    for (int i = 0; i < rayCount; i++) {
        rayPos.x = rays[i].Position.x;
        rayScale.y = rays[i].Scale;
        
        colour = glm::vec4(1.0f) * rays[i].Brightness;
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex);
    }

    for (int i = 2 * rayCount; i < rayArraySize; i++) {
        rayPos.x = rays[i].Position.x;
        rayPos.y = rays[i].Position.y;
        rayScale.y = rays[i].Scale;

        colour = glm::vec4(1.0f) * rays[i].Brightness;
        colour.a = 1.0f;

        Core::Renderer2D::DrawTextureQuad(rayPos, rayScale, colour, rays[i].TexPosition, texScale, rays[i].Atlasindex);
    }

    Core::Renderer2D::EndScene();

    Core::UI::Begin({ 0,0 }, { m_ViewPortWidth, m_ViewPortHeight }, Core::UI::LayoutType::Vertical, glm::vec4(0.0f));
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

    Core::UI::Begin({ 0.0f, 0.0f }, { m_ViewPortWidth, m_ViewPortHeight }, Core::UI::LayoutType::Horizontal, glm::vec4(0.0f));
    
    Core::UI::Text(frameStats, 0.5f, Core::UI::PositioningType::Relative, {-0.495f, -0.47f}, {0.125f, 0.075f}, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));

    Core::UI::BeginContainer({ 0.5, 1.0f }, glm::vec4(0.0f));
        if (Core::UI::Button("Swtich device", { 0.5f, 0.125f })) {
            Core::Audio::GetDevices();
            Core::Audio::SetDevice(0);
        }
        
        static Core::Audio::Index soundIndex = Core::Audio::GetSoundIndex("sound");
        //Core::Audio::ValidateSoundIndex(soundIndex, "sound");
        Core::Audio::Sound& sound = *Core::Audio::GetSound(soundIndex);
        
        if (Core::UI::Button("Play", { 0.5f, 0.125f })) {
            sound.Start();
        }

        if (Core::UI::Button("Play fade in", { 0.5f, 0.125f })) {
            sound.Start(std::chrono::milliseconds(1000));
        }
        
        if (Core::UI::Button("Fade out", { 0.5f, 0.125f })) {
            sound.SetFadeOut(std::chrono::milliseconds(5000));
        }

        if (Core::UI::Button("Re", { 0.5f, 0.125f })) {
            sound.Reinit();
        }
    Core::UI::EndContainer();

    Core::UI::BeginContainer({ 0.5f, 0.25f }, glm::vec4(0.0f));
    static auto foo = Core::Audio::GetDevices();
    for (size_t i = 0; i < foo.size(); i++) {
        Core::UI::Text(foo[i], Core::UI::PositioningType::Offset, { -0.25f, 0.0f }, { 1.0f, 0.25f });
    }
    
    float time = sound.GetTime().count() / 1000.0f;
    float fade = sound.GetFadeVolume();
    std::wstring timeText = std::to_wstring(time) + L"s\n" + std::to_wstring(fade) + L" volume";
    Core::UI::Text(timeText, Core::UI::PositioningType::Offset, {-0.25f, 0.0f}, {1.0f, 0.5f});

    Core::UI::EndContainer();

    Core::UI::End(deltaTime);
}

void RaycasterLayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&RaycasterLayer::OnWindowResizeEvent, this, std::placeholders::_1));

    Core::UI::OnEvent(event);
}

bool RaycasterLayer::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() * 0.5f;
    m_ViewPortHeight = event.GetHeight();

    return false;
}

void Layer2D::OnUpdate(Core::Timestep deltaTime) {
    static glm::vec3 AxisZ(0.0f, 0.0f, 1.0f);
    static glm::vec3 zero(0.0f);
    static glm::mat4 identity(1.0f);
    
    Core::RenderAPI::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer2D::BeginScene(m_Scene->GetCamera());

    const std::vector<Core::Tile>& tiles = m_Scene->GetTiles();
    uint32_t mapSize = tiles.size();

    for (int i = 0; i < mapSize; i++) {
        if (tiles[i].IsTriangle) {
            Core::Renderer2D::DrawRotatedFlatTriangle(tiles[i].Posistion, tiles[i].Rotation, AxisZ, tiles[i].Scale, { tiles[i].Colour.x, tiles[i].Colour.y, tiles[i].Colour.z, 1.0f });
        }
        else {
            Core::Renderer2D::DrawFlatQuad(tiles[i].Posistion, tiles[i].Scale, { tiles[i].Colour.x, tiles[i].Colour.y, tiles[i].Colour.z, 1.0f });
        }
    }

    Core::Renderer2D::EndScene();
    Core::Renderer2D::BeginScene(identity);

    glm::vec4 colour = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    const Core::Player& player = m_Scene->GetPlayer();
    Core::Renderer2D::DrawRotatedFlatQuad(zero, player.Rotation, AxisZ, player.Scale, colour);

    colour = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    std::vector <Core::Line> lines = m_Scene->GetLines();
    uint32_t lineCount = lines.size();
    for (int i = 0; i < lineCount; i++) {
        Core::Renderer2D::DrawLine(lines[i].Posistion, lines[i].Scale, colour);
    }

    Core::Renderer2D::EndScene();
}

void Layer2D::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&Layer2D::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool Layer2D::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() * 0.5f;
    m_ViewPortHeight = event.GetHeight();

    return false;
}

void Layer3D::OnUpdate(Core::Timestep deltaTime) {
    glm::mat4 viewPerspective = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 500.0f) * m_Scene->GetCamera3D().GetViewMatrix();
    
    Core::RenderAPI::SetViewPort(m_ViewPortWidth, 0, m_ViewPortWidth, m_ViewPortHeight);
    Core::Renderer::BeginScene(viewPerspective);

    const std::vector<Core::Model>& models = m_Scene->GetModels();

    for (const Core::Model& model : models) {
        Core::Renderer::DrawModel(model);
    }
}

void Layer3D::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&Layer3D::OnWindowResizeEvent, this, std::placeholders::_1));
}

bool Layer3D::OnWindowResizeEvent(Core::WindowResize& event) {
    m_ViewPortWidth = event.GetWidth() * 0.5f;
    m_ViewPortHeight = event.GetHeight();

    return false;
}
