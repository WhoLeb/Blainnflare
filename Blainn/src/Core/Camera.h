#pragma once

#include "SimpleMath.h"

namespace Blainn
{
	class Camera
	{
	public:
		Camera(
			float fov = 60.f,
			float aspectRatio = 16.f/9.f,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		);
		explicit Camera(
			float fov = 60.f,
			int width = -1,
			int height = -1,
			float nearPlane = 0.1f,
			float farPlane = 1000.f
		);

		void SetPosition(const DirectX::SimpleMath::Vector3& pos);
		void SetRotation(const DirectX::SimpleMath::Vector3& pos);

		void SetPositionAndRotation(const DirectX::SimpleMath::Vector3& pos, const DirectX::SimpleMath::Vector3& rot);

		void SetViewportDimentions(float width, float height);

		DirectX::SimpleMath::Matrix GetViewMatrix() const { return m_ViewMatrix; }
		DirectX::SimpleMath::Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; }

		DirectX::SimpleMath::Vector3 GetPosition() const { return m_Position; }
		int GetViewportWidth() const { return m_ViewportWidth; }
		int GetViewportHeight() const { return m_ViewportHeight; }
		float GetNearPlane() const { return m_NearPlane; }
		float GetFarPlane() const { return m_FarPlane; }
	private:
		void UpdateViewMatrix();
		void UpdateProjectionMatrix();

	private:
		DirectX::SimpleMath::Vector3 m_Position;
		DirectX::SimpleMath::Vector3 m_Rotation;

		float m_FOV;
		float m_AspectRatio;
		float m_NearPlane;
		float m_FarPlane;

		int m_ViewportWidth = -1;
		int m_ViewportHeight = -1;

		DirectX::SimpleMath::Matrix m_ViewMatrix = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Matrix m_ProjectionMatrix = DirectX::SimpleMath::Matrix::Identity;
	};
}
