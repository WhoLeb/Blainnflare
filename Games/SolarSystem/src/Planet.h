#pragma once

#include "Scene/Actor.h"

#include <string>

namespace solar
{
	class Planet : public Blainn::Actor
	{
		using Super = Blainn::GameObject;
	public:
		enum Direction
		{
			DIRClockwise = -1,
			DIRCounterClockwise = 1
		};

		Planet(
			const std::string& name = "planet",
			float radius = 3.f,
			float startingAngle = 0.f,
			float period = 1.f,
			Direction direction = DIRClockwise,
			float rotationSpeed = 10.f
		)
			: m_name(name)
			, m_Radius(radius)
			, m_CurrentAngle(DirectX::XMConvertToRadians(startingAngle))
			, m_OrbitalPeriod(period)
			, m_Direction(direction)
			, m_RotationSpeed(rotationSpeed)
		{}

		void OnUpdate(const Blainn::GameTimer& gt) override;

		const std::string& GetName() const { return m_name; }

	private:
		float m_Radius = 10.f;
		float m_CurrentAngle = 0.f;
		float m_OrbitalPeriod = 1.f;

		Direction m_Direction = DIRClockwise;

		float m_RotationSpeed = 10.f;

		std::string m_name;
	};
}
