#pragma once

#include "Core/Serialization/Archive.h"

namespace Settings::Input {
    inline bool s_FreeLook = true;
    inline bool s_MouseLook = true;
}

namespace Settings {
    bool Serialize(Core::Serialization::Archive& archive);
    bool Deserialize(Core::Serialization::Archive& archive);
}