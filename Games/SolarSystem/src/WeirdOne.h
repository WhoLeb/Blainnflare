#pragma once

#include "Scene/Actor.h"

#include <string>

namespace solar
{
	class WeirdOne : public Blainn::Actor
	{
	public:
		WeirdOne(const std::string& name = "weird one") : m_Name(name) {}
		void OnUpdate(const Blainn::GameTimer& gt) override;

	private:
		std::string m_Name;
	};
}
