#pragma once

#include "SavedState.h"

#include "Core/Serialization/Archive.h"

namespace Settings::Input {
    inline SavedState<bool> s_FreeLook = true;
    inline SavedState<bool> s_MouseLook = true;
}

namespace Settings {
    bool Serialize(Core::Serialization::Archive& archive);
    bool Deserialize(Core::Serialization::Archive& archive);
}