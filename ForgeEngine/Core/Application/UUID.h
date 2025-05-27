#pragma once
#include <cstdint>

namespace ForgeEngine {

    class UUID
    {
    public:
        UUID();
        UUID(uint64_t uuid);
        UUID(const UUID&) = default;

        operator uint64_t() const { return m_UUID; }
    private:
        uint64_t m_UUID;
    };

}


namespace std {

    template<>
    struct hash<ForgeEngine::UUID>
    {
        std::size_t operator()(const ForgeEngine::UUID& uuid) const noexcept
        {
            return static_cast<uint64_t>(uuid);
        }
    };

}
