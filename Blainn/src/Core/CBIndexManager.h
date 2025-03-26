#pragma once

#include "UUID.h"

#include <stack>
#include <stdexcept>
#include <unordered_map>

extern const UINT32 g_NumObjects;

namespace Blainn
{
	class CBIndexManager
	{
	public:
		static CBIndexManager& Get()
		{
			static CBIndexManager instance;
			return instance;
		}

		UINT32 GetCBIdx(UUID uuid)
		{
			auto it = m_UUIDToCBIndex.find(uuid);
			return (it != m_UUIDToCBIndex.end()) ? it->second : UINT32_MAX;
		}

		UINT32 AssignCBIdx(UUID uuid)
		{
			UINT32 idx = GetCBIdx(uuid);
			if (idx != UINT32_MAX)
				return idx;

			if (m_FreeBufferIndices.empty())
				throw std::runtime_error("No free constant buffer indices available");

			UINT32 bufferIndex = m_FreeBufferIndices.top();
			m_FreeBufferIndices.pop();
			m_UUIDToCBIndex[uuid] = bufferIndex;

			return bufferIndex;
		}

		void ReleaseCBIdx(UUID uuid)
		{
			auto it = m_UUIDToCBIndex.find(uuid);
			if (it != m_UUIDToCBIndex.end())
			{
				UINT32 bufferIndex = it->second;
				m_FreeBufferIndices.push(bufferIndex);
				m_UUIDToCBIndex.erase(it);
			}
			else 
				OutputDebugStringW(L"This uuid was not assigned a constant buffer index!");
		}

	private:
		CBIndexManager()
		{
			for (UINT32 i = g_NumObjects - 1; i != 0; i--)
			{
				m_FreeBufferIndices.push(i);
			}
		}

		std::unordered_map<UUID, UINT32> m_UUIDToCBIndex;
		std::stack<UINT32> m_FreeBufferIndices;

	};
}
