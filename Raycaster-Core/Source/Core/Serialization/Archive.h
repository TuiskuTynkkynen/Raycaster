#pragma once

#include <span>
#include <cstddef>
#include <string_view>
#include <memory>
#include <filesystem>
#include <optional>

namespace Core::Serialization {
    class Archive;

    template <typename T>
    concept Serializable = requires(T a, Archive & archive)
    {
        Deserialize<T>(archive);
        Serialize<T>(a, archive);
    };

    class Archive {
    public:
        Archive(std::string_view fileName);
        explicit Archive(std::filesystem::path file);
        Archive(Archive&&) = default;
        ~Archive();

        bool IsGood() const;
        inline size_t GetSize() const { return m_Size; }
        size_t GetPosition() const;
        bool SeekPosition(size_t position);

        bool ReadBytes(std::span<std::byte>);
        template <Serializable T>
        std::optional<T> Read() { return Deserialize<T>(*this); }

        bool WriteBytes(std::span<const std::byte>);
        template <Serializable T>
        bool Write(const T& value) { return Serialize(value, *this); }

        void TruncateFile();
    private:
        std::filesystem::path m_Path;
        std::unique_ptr<std::basic_fstream<std::byte>> m_Stream;
        size_t m_Size;
    };
}