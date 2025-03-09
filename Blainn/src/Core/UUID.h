#pragma once

#include "basetsd.h"
#include <memory>

namespace Blainn
{
	class UUID
	{
	public:
		UUID();
		UUID(UINT64 uuid);
		UUID(const UUID& other);

		operator UINT64() { return m_UUID; }
		operator const UINT64() const { return m_UUID; }

	private:
		UINT64 m_UUID;
	};

	class UUID32
	{
	public:
		UUID32();
		UUID32(UINT32 uuid);
		UUID32(const UUID32& other);

		operator UINT32 () { return m_UUID; }
		operator const UINT32() const { return m_UUID; }
	private:
		UINT32 m_UUID;
	};

}

namespace std {

	template <>
	struct hash<Blainn::UUID>
	{
		std::size_t operator()(const Blainn::UUID& uuid) const
		{
			// uuid is already a randomly generated number, and is suitable as a hash key as-is.
			// this may change in future, in which case return hash<uint64_t>{}(uuid); might be more appropriate
			return uuid;
		}
	};

	template <>
	struct hash<Blainn::UUID32>
	{
		std::size_t operator()(const Blainn::UUID32& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}
	};
}
