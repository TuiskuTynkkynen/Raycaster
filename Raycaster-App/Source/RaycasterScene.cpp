#include "RaycasterScene.h"

#include "UILayer.h"
#include "RaycasterLayer.h"
#include "Layer2D.h"
#include "Layer3D.h"

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
    ShutdownSystems();
    SetState(State::Invalid);

    Core::Audio::Shutdown();
}

void RaycasterScene::ShutdownSystems() {
    m_Player.Shutdown();
    m_Interactables.Shutdown();
    m_Enemies.Shutdown();
    m_Projectiles.Shutdown();
    m_Renderables.Shutdown();
}

void RaycasterScene::Reinit() {
    ShutdownSystems();
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

    m_Interactables.Add(InteractableType::ChestEmpty,  {  3.1f,  2.5f }); // Storage Room
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
    m_Enemies.Add(EnemyType::Basic, { 17.5f, 5.5f }); // Cross Room
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
    if (m_State != State::Running) {
        return;
    }
    
    m_Renderables.ResetDynamic();
    m_Map.Update(deltaTime, m_Lights);
    
    if (m_Player.ShouldInteract()) {
        auto result = m_Interactables.Interact(m_Player.GetPosition(), m_Player.GetYaw());

        switch (result.GetType()) {
        case InteractionResult::Type::Debug:
            RC_TRACE("{}", std::get<std::string_view>(result.Data));
            break;
        case InteractionResult::Type::Toggle:
            m_Map.ToggleDoor(std::get<glm::vec2>(result.Data));
            break;
        case InteractionResult::Type::Pickup:
            m_Player.PickUp(std::get<Item>(result.Data));
            break;
	case InteractionResult::Type::Spawn:
        case InteractionResult::Type::None:
            break;
        }
    }

    {
        auto attacks = m_Player.GetAttacks();
        for (auto& attack : attacks) {
            m_Enemies.DamageAreas(attack.Areas, attack.Thickness, attack.Damage);
        }
        auto projectiles = m_Player.GetProjectiles();
        for (auto& projectile : projectiles) {
            m_Projectiles.Add(projectile.Type, projectile.Position, projectile.Velocity);
        }
    }

    m_Player.Update(m_Walls, m_Map.GetDoors(), deltaTime);
    m_Player.UpdateRender(m_Renderables);

    m_Camera->UpdateCamera(m_Player.GetPosition(), m_Player.GetYaw(), m_Player.GetPitch());
    m_Camera3D->UpdateCamera(glm::vec3(m_Player.GetPosition().x, m_Player.GetPosition().z, m_Player.GetPosition().y), -m_Player.GetYaw(), m_Player.GetPitch());

    m_Projectiles.Update(deltaTime, m_Map);
    for (size_t i = 0; i < m_Projectiles.Count(); i++) {
        auto& projectile = m_Projectiles[i];

        std::array area = { LineCollider(projectile.Position, projectile.Position + 1e-5f) };
        bool hit = m_Enemies.DamageAreas(area, 1e-5f, projectile.Damage);

        hit |= m_Player.DamageAreas(area, 1e-5f, projectile.Damage);

        if (hit) {
            m_Projectiles.Remove(i--);
        }
    }

    m_Projectiles.UpdateRender(m_Renderables, m_Player.GetPosition());

    m_Enemies.Update(deltaTime, m_Map, m_Player.GetPosition());
    {
    auto attacks = m_Enemies.GetAttacks();
    for (auto& attack : attacks) {
        m_Player.DamageAreas(attack.Areas, attack.Thickness, attack.Damage);
    }

        auto projectiles = m_Enemies.GetProjectiles();
        for (auto& projectile : projectiles) {
            m_Projectiles.Add(projectile.Type, projectile.Position, projectile.Velocity);
        }
    }
    m_Enemies.UpdateRender({ m_Tiles.end() - m_Enemies.Count(), m_Tiles.end()}, m_Renderables);

    m_Interactables.Update(deltaTime);
    m_Interactables.UpdateRender(m_Renderables);

    m_Renderer.Render(m_Map, *m_Camera.get(), m_Player, m_Renderables);
    m_Renderables.UpdateDynamicRender(m_Player.GetYaw() - 90.0f);

    if (m_Player.GetHealth() <= 0.0f) {
        SetState(State::Dead);
    }

    // Render door on 3D-layer
    for (const auto& door : m_Map.GetDoors()) {
        auto& model = m_Renderables.GetNextModel();
        int8_t atlasIndex = glm::abs(m_Map[m_Map.GetIndex(door.Position)]);

        glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
        model.Materials.front()->Parameters.back().Value = glm::vec2(0.0f, 0.0f);
        model.Materials.front()->Parameters.front().Value = index;

        glm::vec2 position = door.Position + door.Vector * (door.Length - 0.5f);
        glm::vec2 offset = 0.125f * glm::sign(glm::vec2(m_Player.GetPosition()) - door.Position)
            * glm::vec2(door.Vector.y, door.Vector.x);
        
        glm::vec3 position3D(position.x + offset.x, 0.5f, position.y + offset.y);
        model.Transform = glm::translate(glm::mat4(1.0f), position3D);

        bool backSide = (offset.x + offset.y) < 0.0f;
        float rot = backSide * 180.f + (door.Vector.y != 0) * 90.0f;
        model.Transform = glm::rotate(model.Transform, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void RaycasterScene::SetState(State state) {
    m_State = state;

    auto mode = m_State == State::Running ?
        Core::Input::CursorMode::Disabled : Core::Input::CursorMode::Normal;
    Core::Input::SetCursorMode(mode);
}

bool RaycasterScene::OnRestart(Restart& event) {
    Reinit();

    return true;
}

bool RaycasterScene::OnResume(Resume& event) {
    RC_ASSERT(m_State == State::Paused, "Scene must be in paused state to unpause");
    SetState(State::Running);

    return true;
}

bool RaycasterScene::OnKeyReleased(Core::KeyReleased& event) {
    if (event.GetKeyCode() != RC_KEY_ESCAPE) {
        return false;
    }

    if (m_State <= State::Paused) {
        SetState(m_State == State::Running ? State::Paused : State::Running);
    }

    return true;
}

void RaycasterScene::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Restart>([this](Restart& event) { return OnRestart(event); });
    dispatcer.Dispatch<Resume>([this](Resume& event) { return OnResume(event); });
    dispatcer.Dispatch<Core::KeyReleased>([this](Core::KeyReleased& event) { return OnKeyReleased(event); });

    if (m_State == State::Running && !event.Handled) {
        m_Player.OnEvent(event);
    }
}
