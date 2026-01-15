#include "Archive.h"

#include "Platform.h"
#include "Core/Debug/Debug.h"

#include <fstream>
#include <filesystem>

namespace Core::Serialization {
    Archive::Archive(std::string_view fileName) {
        std::filesystem::path path(fileName);
        if (!path.is_absolute()) {
            path = ApplicationDirectory() / fileName;
        }

        // Ensure file exists
        if (!std::filesystem::exists(path)) {
            std::ofstream(path.c_str());
        }

        using ios = std::ios_base;
        m_Stream = std::make_unique<std::basic_fstream<std::byte>>(path.c_str(), ios::binary | ios::ate | ios::in | ios::out);
        m_Size = m_Stream->tellg();
        m_Stream->seekp(0);
    }

    Archive::Archive(std::filesystem::path file) {
        RC_ASSERT(std::filesystem::is_directory(file), "Tried to create a Serialization Archive from a directoty");

        // Ensure file exists
        if (!std::filesystem::exists(file)) {
            std::ofstream(file.c_str());
        }

        using ios = std::ios_base;
        m_Stream = std::make_unique<std::basic_fstream<std::byte>>(file.c_str(), ios::binary | ios::ate | ios::in | ios::out);
        m_Size = m_Stream->tellg();
        m_Stream->seekp(0);
    }

    Archive::~Archive() {}

    bool Archive::IsGood() const {
        return m_Stream->good();
    }

    size_t Archive::GetPosition() const {
        return m_Stream->tellp();
    }

    bool Archive::SeekPosition(size_t position) {
        if (position >= m_Size) {
            RC_WARN("Tried to seek to position, {}, but file size is {}", position, m_Size);
        }

        m_Stream->seekp(position, std::ios_base::beg);
        return !m_Stream->bad();
    }

    bool Archive::Read(std::span<std::byte> buffer) {
        m_Stream->read(buffer.data(), buffer.size());
        return m_Stream->gcount() == buffer.size();
    }

    bool Archive::Write(std::span<const std::byte> data) {
        m_Stream->write(data.data(), data.size());

        if (auto size = m_Stream->tellp(); size != std::streampos(-1)) {
            m_Size = std::max(m_Size, GetPosition());
            return true;
        }

        return false;
    }
}