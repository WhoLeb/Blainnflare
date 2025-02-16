#pragma once

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
