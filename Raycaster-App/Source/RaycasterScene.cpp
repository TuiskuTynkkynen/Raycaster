#include "RaycasterScene.h"

#include "UILayer.h"
#include "RaycasterLayer.h"
#include "Layer2D.h"
#include "Layer3D.h"

#include "Core/Base/Application.h"
#include "Core/Audio/Audio.h"
#include "Core/Renderer/RenderAPI.h"

RaycasterScene::RaycasterScene() {
    m_Lights.push_back(glm::vec3( 4.0f,  2.1f, 0.75f)); // Storage Room
    m_Lights.push_back(glm::vec3( 7.0f,  2.5f, 0.75f)); // Cross Room
    m_Lights.push_back(glm::vec3(17.5f,  5.5f, 0.75f)); // Cross Room
    m_Lights.push_back(glm::vec3( 1.5f,  7.5f, 0.75f)); // Closet
    m_Lights.push_back(glm::vec3( 3.5f,  9.0f, 0.75f)); // Ruin Hallway
    m_Lights.push_back(glm::vec3( 7.5f,  9.5f, 0.75f)); // Dagger Room
    m_Lights.push_back(glm::vec3(11.5f,  9.5f, 0.75f)); // ???
    m_Lights.push_back(glm::vec3( 2.5f, 12.5f, 0.75f)); // Start Area
    m_Lights.push_back(glm::vec3(10.0f, 18.5f, 0.75f)); // Start Area
    m_Lights.push_back(glm::vec3( 2.5f, 19.5f, 0.75f)); // Spawn
    
    m_Lights.push_back(glm::vec3(18.5f, 12.5f, 0.5f)); // Ambient 1
    m_Lights.push_back(glm::vec3( 7.5f, 18.5f, 0.5f)); // Ambient 2

    m_Map.CalculateLightMap(m_Lights);
    m_Tiles = m_Map.CreateTiles();
    m_Walls = m_Map.CreateWalls();

    Tile tile;
    tile.Colour = glm::vec3(0.0f, 1.0f, 0.0f);
    tile.Scale = glm::vec3(0.4f);
    m_Tiles.insert(m_Tiles.end(), 13, tile);

    m_Camera = std::make_unique<RaycasterCamera>(m_Player.GetPosition(), glm::sqrt(2.0f) / glm::sqrt((float)m_Map.GetSize()), m_Player.GetYaw(), m_Player.GetPitch());
    m_Camera3D = std::make_unique<Core::FlyCamera>(glm::vec3(m_Player.GetPosition().x, 0.5f, m_Player.GetPosition().y), glm::vec3(0.0f, 1.0f, 0.0f), -m_Player.GetYaw() - m_Player.GetPitch());
}

void RaycasterScene::OnAttach(Core::Application& app) {
    app.RequestOverlay<UILayer>();
    app.RequestLayer<RaycasterLayer>();
    app.RequestLayer<Layer3D>();
    app.RequestLayer<Layer2D>();

    Core::Audio::Init();
    Core::Audio::SetWorldUp(glm::vec3(0.0f, 0.0f, 1.0f));
    Core::RenderAPI::SetClearColour(glm::vec3(0.05f, 0.075f, 0.1f));

    Reinit();
}

void RaycasterScene::OnDetach(Core::Application&) {
    Shutdown();
    Core::Audio::Shutdown();
}

void RaycasterScene::Shutdown() {
    SetState(State::Invalid);

    m_Player.Shutdown();
    m_Interactables.Shutdown();
    m_Enemies.Shutdown();
    m_Projectiles.Shutdown();
    m_Renderables.Shutdown();
}

void RaycasterScene::Reinit() {
    Shutdown();
    SetState(State::Running);

    m_Map.Reinit(m_Lights);
    m_Player.Init(m_Map, { 2.5f, 19.5f });
    m_Projectiles.Init();

    m_Interactables.Init();
    m_Interactables.Add(InteractableType::Barrel, { 2.3f, 1.5f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 4.0f, 1.5f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 1.8f, 1.8f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 4.5f, 2.0f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 1.5f, 2.5f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 4.5f, 3.5f });  // Storage Room
    m_Interactables.Add(InteractableType::Barrel, { 1.5f, 7.5f });  // Closet
    m_Interactables.Add(InteractableType::Barrel, { 1.5f, 17.7f }); // Spawn
    m_Interactables.Add(InteractableType::Barrel, { 3.5f, 17.7f }); // Spawn
    m_Interactables.Add(InteractableType::Barrel, { 1.5f, 18.5f }); // Spawn
    m_Interactables.Add(InteractableType::Barrel, { 3.5f, 18.5f }); // Spawn
    m_Interactables.Add(InteractableType::Barrel, { 1.5f, 19.3f }); // Spawn
    m_Interactables.Add(InteractableType::Barrel, { 3.5f, 19.3f }); // Spawn

    m_Interactables.Add(InteractableType::ChestChalice,{  3.1f,  2.5f }); // Storage Room
    m_Interactables.Add(InteractableType::ChestEmpty,  {  1.5f,  5.5f }); // Bait
    m_Interactables.Add(InteractableType::ChestDarts,  { 17.5f,  5.5f }); // Cross
    m_Interactables.Add(InteractableType::ChestDagger, {  7.5f,  9.5f }); // Dagger 
    m_Interactables.Add(InteractableType::ChestDarts,  { 19.5f, 19.5f }); // Secret

    m_Interactables.Add(InteractableType::Corpse, {  8.5f, 20.5f }); // Hedge

    for (glm::vec3 light : m_Lights) {
        if (light.z == 0.5f) { break; }
        m_Interactables.Add(InteractableType::Light, light);
    }

    for (const auto& door : m_Map.GetDoors()) {
        glm::vec2 postion = door.Position + 0.5f * door.Vector;
        m_Interactables.Add(InteractableType::DoorToggle, postion);
    }

    m_Enemies.Init(m_Map);
    m_Enemies.Add(EnemyType::Basic, { 19.5f, 1.5f }); // Cross Room
    m_Enemies.Add(EnemyType::Basic, { 2.5f, 2.5f });  // Storage Room
    m_Enemies.Add(EnemyType::Basic, { 3.5f, 2.5f });  // Storage Room
    m_Enemies.Add(EnemyType::Basic, { 1.5f, 7.5f });  // Closet
    m_Enemies.Add(EnemyType::Basic, { 17.5f, 6.0f }); // Cross Room
    m_Enemies.Add(EnemyType::Basic, { 21.5f, 12.5 }); // Ruins Entrance
    m_Enemies.Add(EnemyType::Basic, { 14.5f, 13.5 }); // Hedge
    m_Enemies.Add(EnemyType::Basic, { 9.5f, 18.5 });  // Start Area

    m_Enemies.Add(EnemyType::Ranged, glm::vec2(7.5f, 2.0f));  // Cross Room
    m_Enemies.Add(EnemyType::Ranged, glm::vec2(7.5f, 3.0f));  // Cross Room
    m_Enemies.Add(EnemyType::Ranged, glm::vec2(10.5f, 6.5f)); // Ruins Ambush
    m_Enemies.Add(EnemyType::Ranged, glm::vec2(3.5f, 9.5f));  // Ruins Hallway
    m_Enemies.Add(EnemyType::Ranged, glm::vec2(1.5f, 22.5f)); // Hedge Ambush
    
    auto shader = std::make_shared<Core::Shader>("Assets/Shaders/3DAtlasShader.glsl");
    //setup shader
    {
        shader->Bind();
        shader->setInt("Texture", 0);
        shader->setInt("MapTexture", 2);
        shader->setInt("AmbientTexture", 3);

        glm::uvec2 atlasSize(ATLASWIDTH, ATLASHEIGHT);
        shader->setVec2("AtlasSize", atlasSize);

        RC_ASSERT(m_Lights.size() < std::numeric_limits<uint32_t>::max());

        const uint32_t lightCount = glm::min(static_cast<uint32_t>(m_Lights.size()), 10u);
        shader->setInt("LightCount", lightCount);
        for (uint32_t i = 0; i < lightCount; i++) {
            std::string lightName = "PointLights[i]";
            lightName[12] = '0' + i;
            glm::vec3 pos(m_Lights[i].x, m_Lights[i].z, m_Lights[i].y);
            shader->setVec3(lightName.c_str(), pos);
        }

        const uint32_t maxAmbientIndex = glm::min(static_cast<uint32_t>(m_Lights.size()), lightCount + 10u);
        shader->setInt("AmbientCount", maxAmbientIndex - lightCount);
        for (uint32_t i = lightCount; i < maxAmbientIndex; i++) {
            std::string lightName = "AmbientLights[i]";
            lightName[14] = '0' + i - lightCount;
            glm::vec3 pos(m_Lights[i].x, m_Lights[i].z, m_Lights[i].y);
            shader->setVec3(lightName.c_str(), pos);
        }
    }
    
    auto textureAtlas = std::make_shared<Core::Texture2D>(Core::Texture2D::WrapMode::Repeat, Core::Texture2D::WrapMode::Repeat, Core::Texture2D::Filter::Nearest, Core::Texture2D::Filter::Nearest);
    textureAtlas->BindImage("Assets/Textures/atlas.png");
    
    m_Renderables.Init(shader, textureAtlas, m_Map.GetMapTexture());
    m_Renderables.PushStaticModel(m_Map.CreateModel(m_Walls, textureAtlas, shader)); // Map
    m_Renderables.PushStaticModel(); // Hand / Held Item
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    Core::RenderAPI::Clear();
}
