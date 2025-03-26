#pragma once

#include "UUID.h"

#include <unordered_map>
#include <stack>

extern const UINT32 g_NumObjects;

namespace Blainn
{

	class MaterialIndexManager
	{
	public:
		static MaterialIndexManager& Get()
		{
			static MaterialIndexManager instance;
			return instance;
		}

		UINT32 GetMatIdx(UUID uuid)
		{
			auto it = m_UUIDtoIdx.find(uuid);
			return (it != m_UUIDtoIdx.end()) ? it->second : UINT32_MAX;
		}

		UINT32 AssignCBIdx(UUID uuid)
		{
			UINT32 idx = GetMatIdx(uuid);
			if (idx != UINT32_MAX)
				return idx;

			if (m_FreeStack.empty())
				throw std::runtime_error("No free constant buffer indices available");

			UINT32 bufferIndex = m_FreeStack.top();
			m_FreeStack.pop();
			m_UUIDtoIdx[uuid] = bufferIndex;

			return bufferIndex;
		}

		void ReleaseCBIdx(UUID uuid)
		{
			auto it = m_UUIDtoIdx.find(uuid);
			if (it != m_UUIDtoIdx.end())
			{
				UINT32 bufferIndex = it->second;
				m_FreeStack.push(bufferIndex);
				m_UUIDtoIdx.erase(it);
			}
			else 
				OutputDebugStringW(L"This uuid was not assigned a constant buffer index!");
		}

	private:
		MaterialIndexManager()
		{
			for (auto i = g_NumObjects; i != 0; i--)
				m_FreeStack.push(i);
		}

		std::unordered_map<UUID, UINT32> m_UUIDtoIdx;
		std::stack<UINT32> m_FreeStack;
	};

}
