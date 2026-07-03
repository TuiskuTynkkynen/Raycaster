#include "GameScene.h"

void GameScene::OnUpdate(Core::Timestep deltaTime) {
    RaycasterScene::OnUpdate(deltaTime);
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
        case InteractionResult::Type::Win:
            SetState(State::Win);
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
    m_Enemies.UpdateRender(m_Tiles, m_Renderables);

    m_Interactables.Update(deltaTime);
    m_Interactables.UpdateRender(m_Renderables);

    m_Renderer.Render(m_Map, *m_Camera.get(), m_Renderables);
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

void GameScene::SetState(State state) {
    m_State = state;

    auto mode = m_State == State::Running ?
        Core::Input::CursorMode::Disabled : Core::Input::CursorMode::Normal;
    Core::Input::SetCursorMode(mode);
}

bool GameScene::OnRestart(Restart& event) {
    Reinit();
    SetState(State::Running);

    return true;
}

bool GameScene::OnResume(Resume& event) {
    RC_ASSERT(m_State == State::Paused, "Scene must be in paused state to unpause");
    SetState(State::Running);

    return true;
}

bool GameScene::OnKeyReleased(Core::KeyReleased& event) {
    if (event.GetKeyCode() != RC_KEY_ESCAPE) {
        return false;
    }

    if (m_State <= State::Paused) {
        SetState(m_State == State::Running ? State::Paused : State::Running);
    }

    return true;
}

void GameScene::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Restart>([this](Restart& event) { return OnRestart(event); });
    dispatcer.Dispatch<Resume>([this](Resume& event) { return OnResume(event); });
    dispatcer.Dispatch<Core::KeyReleased>([this](Core::KeyReleased& event) { return OnKeyReleased(event); });

    if (m_State == State::Running && !event.Handled) {
        m_Player.OnEvent(event);
    }
}
