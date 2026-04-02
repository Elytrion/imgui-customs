#include "UUIDGenerator.h"
#include <rpc.h>
#include <cstdint>

namespace
{
    // -------- Helper functions --------
    //RPC helpers to convert between Windows UUID and RFC4122 byte order
    static inline void pack_win_uuid_bytes(const UUID& u, CustomUUID& out)
    {
        auto& d = out.data;
        d[0] = static_cast<uint8_t>((u.Data1 >> 24) & 0xFF);
        d[1] = static_cast<uint8_t>((u.Data1 >> 16) & 0xFF);
        d[2] = static_cast<uint8_t>((u.Data1 >> 8) & 0xFF);
        d[3] = static_cast<uint8_t>(u.Data1 & 0xFF);
        d[4] = static_cast<uint8_t>((u.Data2 >> 8) & 0xFF);
        d[5] = static_cast<uint8_t>(u.Data2 & 0xFF);
        d[6] = static_cast<uint8_t>((u.Data3 >> 8) & 0xFF);
        d[7] = static_cast<uint8_t>(u.Data3 & 0xFF);
        std::memcpy(&d[8], u.Data4, 8);
    }
    // Unpack CustomUUID.data (RFC4122 byte order)
    static inline UUID unpack_win_uuid_bytes(const CustomUUID& in)
    {
        const auto& d = in.data;
        UUID u{};
        u.Data1 = (unsigned long)((uint32_t(d[0]) << 24) |
            (uint32_t(d[1]) << 16) |
            (uint32_t(d[2]) << 8) |
            uint32_t(d[3]));
        u.Data2 = (unsigned short)((uint16_t(d[4]) << 8) | uint16_t(d[5]));
        u.Data3 = (unsigned short)((uint16_t(d[6]) << 8) | uint16_t(d[7]));
        std::memcpy(u.Data4, &d[8], 8);
        return u;
    }
}

CustomUUID CustomUUID::GenerateUUID()
{
    UUID u{};
    if (UuidCreate(&u) != RPC_S_OK) {
        return {}; // nil on failure
    }
    CustomUUID out{};
    pack_win_uuid_bytes(u, out);
    return out;
}
CustomUUID CustomUUID::FromString(const std::string& uuid)
{
    UUID u{};
    if (UuidFromStringA(reinterpret_cast<RPC_CSTR>(const_cast<char*>(uuid.c_str())), &u) != RPC_S_OK) {
        return {};
    }
    CustomUUID out{};
    pack_win_uuid_bytes(u, out);
    return out;
}
std::string CustomUUID::ToString() const
{
    UUID u = unpack_win_uuid_bytes(*this);
    RPC_CSTR strUuid = nullptr;
    if (UuidToStringA(&u, &strUuid) != RPC_S_OK) {
        return {};
    }
    std::string out(reinterpret_cast<char*>(strUuid));
    RpcStringFreeA(&strUuid);
    return out;
}
