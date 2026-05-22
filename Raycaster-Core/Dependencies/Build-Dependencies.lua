RaycasterCoreDependencies = {}

local base_path = path.getabsolute("./")
local function getabsolutepath(relative_path)
    return base_path  .. "/" .. relative_path
end

function RaycasterCoreDependencies.include()
    includedirs {
        getabsolutepath "glm",
        getabsolutepath "FreeType/include",
        getabsolutepath "miniaudio",
        getabsolutepath "utils",
    }

    links {
        "glm",
        "miniaudio",
        "FreeType",
        "Opusfile",
    }

    filter "system:not emscripten"
        includedirs {
            getabsolutepath "GLFW/include",
            getabsolutepath "glad/include",
        }

        links {
            "GLFW",
            "glad",
        }
    
    filter "system:emscripten"
        libdirs {
            getabsolutepath "libogg/build/lib",
            getabsolutepath "libopus/build/lib",
        }

        links {
            "ogg",
            "opus",
        }

    -- Reset filters
    filter {}
end

group "Core/Dependencies"
    include(getabsolutepath "glm.lua")
    include(getabsolutepath "FreeType.lua")
    include(getabsolutepath "miniaudio.lua")
    include(getabsolutepath "opusfile.lua")
    include(getabsolutepath "stb_image.lua")

    if os.target() ~= "emscripten" then
        include(getabsolutepath "GLFW.lua")
        include(getabsolutepath "glad.lua")
    end
group "Core"
