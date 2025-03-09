#include "pch.h"
#include "Camera.h"

namespace Blainn
{

	Camera::Camera(float fov, float aspectRatio, float nearPlane, float farPlane)
		: m_FOV(fov)
		, m_AspectRatio(aspectRatio)
		, m_NearPlane(nearPlane)
		, m_FarPlane(m_FarPlane)
	{
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	Camera::Camera(float fov, int width, int height, float nearPlane, float farPlane)
		: m_FOV(fov)
		, m_AspectRatio(float(width)/float(height))
		, m_NearPlane(0.1f)
		, m_FarPlane(1000.f)
		, m_ViewportWidth(width)
		, m_ViewportHeight(height)
	{
		UpdateProjectionMatrix();
		UpdateViewMatrix();
	}

	void Camera::SetPosition(const DirectX::SimpleMath::Vector3& pos)
	{
		m_Position = pos;
		UpdateViewMatrix();
	}

	void Camera::SetRotation(const DirectX::SimpleMath::Vector3& rot)
	{
		m_Rotation = rot;
		UpdateViewMatrix();
	}

	void Camera::SetPositionAndRotation(const DirectX::SimpleMath::Vector3& pos, const DirectX::SimpleMath::Vector3& rot)
	{
		m_Position = pos;
		m_Rotation = rot;
		UpdateViewMatrix();
	}

	void Camera::SetViewportDimentions(int width, int height)
	{
		m_AspectRatio = float(width) / float(height);
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		UpdateProjectionMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		using namespace DirectX;
		using namespace SimpleMath;
		Vector3 pos = m_Position;
		Vector3 forward = Vector3(0, 0, 1);
		forward.Normalize();
		
		Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		forward = forward.Transform(forward, rotMatrix);

		Vector3 up{ 0, 1, 0 };
		Vector3 target = pos + forward;
		Matrix view = Matrix::CreateLookAt(pos, target, up);
		m_ViewMatrix = view;
	}

	void Camera::UpdateProjectionMatrix()
	{
		using namespace DirectX;
		using namespace SimpleMath;
		
		Matrix proj = Matrix::CreatePerspectiveFieldOfView(
			XMConvertToRadians(m_FOV), m_AspectRatio, m_NearPlane, m_FarPlane);
		m_ProjectionMatrix = proj;
	}

}
