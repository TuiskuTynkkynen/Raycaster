#include "DemoScene.h"

#include "Core/Base/Application.h"

#include <random>

static glm::vec2 RandomPosition(const Map& map) {
    static constexpr size_t MAX_ITERATIONS = 32;
    static std::uniform_int_distribution<size_t> distX(0, Map::GetWidth() - 1);
    static std::uniform_int_distribution<size_t> distY(0, Map::GetHeight() - 1);
    static std::mt19937 engine(std::random_device{}());

    for (size_t i = 0; i < MAX_ITERATIONS; i++) {
        const size_t x = distX(engine), y = distY(engine), index = map.GetIndex(x, y);
        RC_ASSERT(index < Map::GetSize());

        if (map[index] == 0 && map.GetLight(x, y) >= 0.15f) {
            return { static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f };
        }
    }

    // Fallback to centre of map.
    return { static_cast<float>(Map::GetWidth() + 1) / 2.0f, static_cast<float>(Map::GetHeight() + 1) / 2.0f};
}

void DemoScene::OnUpdate(Core::Timestep deltaTime) {
    RaycasterScene::OnUpdate(deltaTime);
    if (m_State != State::Running) {
        return;
    }

    glm::vec2 viewPosition = m_Camera->GetPosition();

    m_TeleportCooldown -= deltaTime.GetSeconds();
    if (m_TeleportCooldown <= 0.0f) {
        m_TeleportCooldown = s_TeleportIntervalSeconds;
        viewPosition = RandomPosition(m_Map);
    }

    m_Yaw += deltaTime.GetSeconds() * s_YawSpeed;
    if (m_Yaw >= 360.0f) { m_Yaw -= 360.0f; }
    float height = 0.5f + glm::sin(glm::radians(m_Yaw)) * 0.0625f;

    m_Camera->UpdateCamera(glm::vec3(viewPosition, height), m_Yaw, 0.0f);
    m_Camera3D->UpdateCamera(glm::vec3(viewPosition.x, height, viewPosition.y), -m_Yaw, 0.0f);

    m_Renderables.ResetDynamic();
    m_Map.Update(deltaTime, m_Lights);

    m_Enemies.UpdateRender(m_Tiles, m_Renderables);
    m_Interactables.UpdateRender(m_Renderables);

    m_Renderer.Render(m_Map, *m_Camera.get(), m_Renderables);
    m_Renderables.UpdateDynamicRender(m_Yaw - 90.0f);

    // Render door on 3D-layer
    for (const auto& door : m_Map.GetDoors()) {
        auto& model = m_Renderables.GetNextModel();
        int8_t atlasIndex = glm::abs(m_Map[m_Map.GetIndex(door.Position)]);

        glm::vec2 index = glm::vec2((atlasIndex) % ATLASWIDTH, (atlasIndex) / ATLASWIDTH);
        model.Materials.front()->Parameters.back().Value = glm::vec2(0.0f, 0.0f);
        model.Materials.front()->Parameters.front().Value = index;

        glm::vec2 position = door.Position + door.Vector * (door.Length - 0.5f);
        glm::vec2 offset = 0.125f * glm::sign(viewPosition - door.Position)
            * glm::vec2(door.Vector.y, door.Vector.x);

        glm::vec3 position3D(position.x + offset.x, 0.5f, position.y + offset.y);
        model.Transform = glm::translate(glm::mat4(1.0f), position3D);

        bool backSide = (offset.x + offset.y) < 0.0f;
        float rot = backSide * 180.f + (door.Vector.y != 0) * 90.0f;
        model.Transform = glm::rotate(model.Transform, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void DemoScene::SetState(State state) {
    m_State = state;
}

bool DemoScene::OnResume(Resume& event) {
    RC_ASSERT(m_State == State::Paused, "Scene must be in paused state to unpause");
    SetState(State::Running);

    return true;
}

bool DemoScene::OnInputReleased(Settings::KeyBinds::InputCode input) {
    if (auto action = Settings::KeyBinds::InputCodeToKeyBind(input);
        !action || action.value() != Settings::KeyBinds::Interact) {
        return false;
    }
    
    m_TeleportCooldown = 0.0f;
    return true;
}

bool DemoScene::OnKeyReleased(Core::KeyReleased& event) {
    if (OnInputReleased(Settings::KeyBinds::InputCode(event.GetKeyCode()))) {
        return true;
    }
    
    if (event.GetKeyCode() != RC_KEY_ESCAPE) {
        return false;
    }

    if (m_State <= State::Paused) {
        SetState(m_State == State::Running ? State::Paused : State::Running);
    }

    return true;
}

void DemoScene::OnEvent(Core::Event& event) {
    Core::EventDispatcher dispatcer(event);
    dispatcer.Dispatch<Resume>([this](auto& e) { return OnResume(e); });
    dispatcer.Dispatch<Core::KeyReleased>([this](auto& e) { return OnKeyReleased(e); });
    dispatcer.Dispatch<Core::MouseButtonReleased>([this](auto& e) { return OnInputReleased(Settings::KeyBinds::InputCode(e.GetButton())); });
}
