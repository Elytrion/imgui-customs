#pragma once
#include <array>
#include <string>
#include <cstdint>

struct CustomUUID
{
	std::array<std::uint8_t, 16> data{};

	static CustomUUID GenerateUUID();
	static CustomUUID FromString(const std::string& str);
	std::string ToString() const;

	bool IsSet() const
	{
		return data != std::array<std::uint8_t, 16>{};
	}
};

static constexpr CustomUUID NIL{}; // all-zero UUID
inline bool operator==(const CustomUUID& lhs, const CustomUUID& rhs)
{
	return lhs.data == rhs.data;
}

namespace std
{
	template<>
	struct hash<CustomUUID>
	{
		size_t operator () (const CustomUUID& uuid) const
		{
			// Taken from boost include/boost/uuid/uuid.hpp
			std::size_t seed = 0;
			for (std::size_t i = 0; i < uuid.data.size(); i++)
			{
				seed ^= static_cast<std::size_t>(uuid.data[i]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}
			return seed;
		}
	};
}