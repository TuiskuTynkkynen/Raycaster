#include "RaycasterLayer.h"

#include "Core/UI/UI.h"

#include <glm/gtc/matrix_transform.hpp>

float timeDelta = 0;
uint32_t frameCount = 0;
float frameTime = 0;

void RaycasterLayer::OnAttach() {
    Core::Renderer2D::SetTextureAtlas("wolfenstein_texture_atlas.png", 11, 2);

    std::shared_ptr<Core::Font> font = std::make_shared<Core::Font>(false);
    font->AddCharacterRange(' ', '~'); //Printable ASCII
    font->AddCharacterRange(0x00A1, 0x0FF); //Printable Latin-1 Supplement
    font->GenerateAtlas("tiny5/tiny5-Medium.ttf", 8);
    Core::Renderer2D::SetFont(font);

    Core::UI::Init();
}
bool foo = false;
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

    frameCount++;
    timeDelta += deltaTime;
    if (timeDelta >= 0.1f) {
        frameTime = 1000.0f * timeDelta / frameCount;
        frameCount = 0;
        timeDelta = 0.0f;
    }
    
    colour = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
    glm::mat4 projection = glm::ortho(0.0f, (float)m_ViewPortWidth, 0.0f, (float)m_ViewPortHeight);
    Core::Renderer2D::BeginScene(projection);
    std::wstring frameStats = std::to_wstring(int(1000/ frameTime)) + L" FPS\n" + std::to_wstring(frameTime) + L" ms";
    if (foo) {
            Core::Renderer2D::DrawString(frameStats, 5.0f, m_ViewPortHeight - 15.0f, 2.0f, colour);
    }
    Core::Renderer2D::EndScene();

    Core::UI::Begin({ 0, 0 }, { m_ViewPortWidth, m_ViewPortHeight });

    Core::UI::BeginContainer({ 0.75f, 0.2f }, { 0.25f, 0.25f, 0.25f, 0.5f }, Core::UI::LayoutType::Horizontal);
        Core::UI::Button({ 0.4f, 0.75f });
        Core::UI::Button({ 0.4f, 0.75f });
    Core::UI::EndContainer();
    
    Core::UI::BeginContainer({ 0.75f, 0.5f }, { 0.8f, 0.25f, 0.25f, 0.25f }, Core::UI::LayoutType::Horizontal);
        Core::UI::BeginContainer({ 0.45f, 1.0f }, { 0.25f, 0.25f, 0.25f, 0.5f }, Core::UI::LayoutType::Vertical);
            Core::UI::Button({ 1.0f, 0.25f });
            Core::UI::Button({ 1.0f, 0.25f });
            Core::UI::Button({ 1.0f, 0.25f });
        Core::UI::EndContainer();

        Core::UI::BeginContainer({ 0.45f, 1.0f }, { 0.25f, 0.25f, 0.25f, 0.5f }, Core::UI::LayoutType::Vertical);
            Core::UI::Button(Core::UI::PositioningType::Offset, {-0.05f, 0.0f}, { 1.0f, 0.25f });
            Core::UI::Button(Core::UI::PositioningType::Offset, {0.05f, 0.0f}, { 1.0f, 0.25f });
            Core::UI::Button(Core::UI::PositioningType::Offset, {0.0f, -0.05f}, { 1.0f, 0.25f });
        Core::UI::EndContainer();
    Core::UI::EndContainer();

    Core::UI::Button(Core::UI::PositioningType::Relative, { -0.45f, -0.45f }, { 0.075f, 0.075f });
    Core::UI::Button(Core::UI::PositioningType::Absolute, { 0.95f, 0.05f }, { 0.075f, 0.075f });

    foo ^= Core::UI::Button({ 0.75f, 0.2f });
    
    
    Core::UI::End();
}

void RaycasterLayer::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Core::WindowResize>(std::bind(&RaycasterLayer::OnWindowResizeEvent, this, std::placeholders::_1));
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
