#include "RaycasterScene.h"

void RaycasterScene::Init(){
    m_Lights.push_back(glm::vec3(2.5f, 3.0f, 0.75f));
    m_Lights.push_back(glm::vec3(21.5f, 3.0f, 0.75f));
    m_Lights.push_back(glm::vec3(18.5f, 18.0f, 0.75f));
    m_Lights.push_back(glm::vec3(8.5f, 6.5f, 0.75f));

    m_Map.CalculateLightMap(m_Lights);
    m_Tiles = m_Map.CreateTiles();
    m_Walls = m_Map.CreateWalls();
    
    Reinit();

    Tile tile;
    tile.Colour = glm::vec3(0.0f, 1.0f, 0.0f);
    tile.Scale = m_Player.GetScale();
    m_Tiles.push_back(tile);
    m_Tiles.push_back(tile);

    m_Camera = std::make_unique<Core::Camera2D>(m_Player.GetPosition(), m_Player.GetRotation(), glm::sqrt(2.0f) / glm::sqrt((float)m_Map.GetSize()));
    m_Camera3D = std::make_unique<Core::FlyCamera>(glm::vec3(m_Player.GetPosition().x, 0.5f, m_Player.GetPosition().y), glm::vec3(0.0f, 1.0f, 0.0f), -m_Player.GetRotation(), 0.0f);

    Core::RenderAPI::SetClearColour(glm::vec3(0.05f, 0.075f, 0.1f));
}

void RaycasterScene::Shutdown() {
    m_State = State::Invalid;

    m_Player.Shutdown();
    m_Interactables.Shutdown();
    m_Enemies.Shutdown();
    m_Projectiles.Shutdown();
    m_Renderables.Shutdown();
}

void RaycasterScene::Reinit() {
    Shutdown();
    m_State = State::Running;

    m_Player.Init(m_Map);
    m_Projectiles.Init();

    m_Interactables.Init();
    m_Interactables.Add(InteractableType::Barrel, { 3.0f, 2.5f });
    m_Interactables.Add(InteractableType::Barrel, { 2.5f, 2.5f });
    m_Interactables.Add(InteractableType::Chest, { 8.5f, 6.5f });

    for (glm::vec2 light : m_Lights) {
        m_Interactables.Add(InteractableType::Light, light);
    }

    for (const auto& door : m_Map.GetDoors()) {
        glm::vec2 postion = door.Position + 0.5f * door.Vector;
        m_Interactables.Add(InteractableType::DoorToggle, postion);
    }

    m_Enemies.Init(m_Map);
    m_Enemies.Add(EnemyType::Basic, glm::vec2(8.5f, 6.5f));
    m_Enemies.Add(EnemyType::Ranged, glm::vec2(2.5f, 3.0f));

    auto shader = std::make_shared<Core::Shader>("Assets/Shaders/3DAtlasShader.glsl");
    //setup shader
    {
        shader->Bind();
        shader->setInt("Texture", 0);

        RC_ASSERT(m_Lights.size() < std::numeric_limits<uint32_t>::max());
        const uint32_t lightCount = glm::min(static_cast<uint32_t>(m_Lights.size()), 10u);
        glm::uvec2 atlasSize(ATLASWIDTH, ATLASHEIGHT);

        shader->setVec2("AtlasSize", atlasSize);
        shader->setInt("LightCount", lightCount);

        for (uint32_t i = 0; i < lightCount; i++) {
            std::string lightName = "PointLights[i]";
            lightName[12] = '0' + i;
            glm::vec3 pos(m_Lights[i].x, m_Lights[i].z, m_Lights[i].y);
            shader->setVec3(lightName.c_str(), pos);
        }
    }
    
    auto textureAtlas = std::make_shared<Core::Texture2D>(Core::Texture2D::WrapMode::Repeat, Core::Texture2D::WrapMode::Repeat, Core::Texture2D::Filter::Nearest, Core::Texture2D::Filter::Nearest);
    textureAtlas->BindImage("Assets/Textures/wolfenstein_texture_atlas.png");

    m_Renderables.Init(shader, textureAtlas);
    m_Renderables.PushStaticModel(m_Map.CreateModel(m_Walls, textureAtlas, shader)); // Map
}

void RaycasterScene::OnUpdate(Core::Timestep deltaTime) {
    Core::RenderAPI::Clear();
    if (m_State != State::Running) {
        return;
    }
    
    m_Renderables.ResetDynamic();
    m_Map.Update(deltaTime, m_Lights);

    if (m_Player.ShouldInteract()) {
        auto result = m_Interactables.Interact(m_Player.GetPosition(), m_Player.GetRotation());

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

    m_Camera->UpdateCamera(m_Player.GetPosition(), m_Player.GetRotation());
    m_Camera3D->UpdateCamera(glm::vec3(m_Player.GetPosition().x, m_Player.GetPosition().z, m_Player.GetPosition().y), -m_Player.GetRotation());

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
    auto attacks = m_Enemies.GetAttacks();
    for (auto& attack : attacks) {
        m_Player.DamageAreas(attack.Areas, attack.Thickness, attack.Damage);
    }
    m_Enemies.UpdateRender({ m_Tiles.end() - m_Enemies.Count(), m_Tiles.end()}, m_Renderables);

    m_Interactables.Update(deltaTime);
    m_Interactables.UpdateRender(m_Renderables);

    m_Renderer.Render(m_Map, *m_Camera.get(), m_Player, m_Renderables);
    
    if (m_Player.GetHealth() <= 0.0f) {
        m_State = State::Dead;
    }

    // Render door on 3D-layer
    for (const auto& door : m_Map.GetDoors()) {
        auto& model = m_Renderables.GetNextModel();
        int8_t atlasIndex = glm::abs(m_Map[m_Map.GetIndex(door.Position)]);

        glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
        model.Materials.front()->Parameters.back().Value = glm::vec2(0.0f, 0.0f);
        model.Materials.front()->Parameters.front().Value = index;

        glm::vec2 position = door.Position + door.Vector * (door.Length - 0.5f);
        glm::vec2 offset = 0.125f * glm::sign(glm::vec2(m_Player.GetPosition()) - door.Position);
        
        glm::vec3 position3D(position.x + door.Vector.y * offset.x, 0.5f, position.y + door.Vector.x * offset.y);
        model.Transform = glm::translate(glm::mat4(1.0f), position3D);
        float rot = door.Vector.y ? glm::radians(90.0f): glm::radians(0.0f);
        model.Transform = glm::rotate(model.Transform, rot, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

bool RaycasterScene::OnRestart(Restart& event) {
    Reinit();

    return true;
}

bool RaycasterScene::OnResume(Resume& event) {
    RC_ASSERT(m_State == State::Paused, "Scene must be in paused state to unpause");
    m_State = State::Running;

    return true;
}

bool RaycasterScene::OnKeyReleased(Core::KeyReleased& event) {
    if (event.GetKeyCode() != RC_KEY_ESCAPE) {
        return false;
    }

    if (m_State <= State::Paused) {
        m_State = m_State == State::Running ? State::Paused : State::Running;
    }

    return true;
}

bool RaycasterScene::OnWindowResize(Core::WindowResize& event) {
    m_Renderer.SetAspecRatio(event.GetWidth() * 0.5f / event.GetHeight());
    return false;
}

void RaycasterScene::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Restart>([this](Restart& event) { return OnRestart(event); });
    dispatcer.Dispatch<Resume>([this](Resume& event) { return OnResume(event); });
    dispatcer.Dispatch<Core::KeyReleased>([this](Core::KeyReleased& event) { return OnKeyReleased(event); });
    dispatcer.Dispatch<Core::WindowResize>([this](Core::WindowResize& event) { return OnWindowResize(event); });

    dispatcer.Dispatch<Core::KeyPressed>([this](Core::KeyPressed& event) { return m_Player.OnKeyEvent(event); });
    dispatcer.Dispatch<Core::KeyReleased>([this](Core::KeyReleased& event) {  return m_Player.OnKeyEvent(event); });
}