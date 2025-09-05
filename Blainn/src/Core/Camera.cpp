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
		, m_NearPlane(nearPlane)
		, m_FarPlane(farPlane)
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
		m_Quaternion = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(rot);
		UpdateViewMatrix();
	}

	void Camera::SetPositionAndRotation(const DirectX::SimpleMath::Vector3& pos, const DirectX::SimpleMath::Vector3& rot)
	{
		m_Position = pos;

		using namespace DirectX;
		using namespace DirectX::SimpleMath;

		float yawRad = XMConvertToRadians(rot.x);
		float pitchRad = XMConvertToRadians(rot.y);
		float rollRad = XMConvertToRadians(rot.z);

		m_Quaternion = DirectX::SimpleMath::Quaternion::CreateFromYawPitchRoll(yawRad, pitchRad, rollRad);
		m_Quaternion.Normalize();
		UpdateViewMatrix();
	}

	void Camera::SetPositionAndQuaternion(const DirectX::SimpleMath::Vector3& pos, const DirectX::SimpleMath::Quaternion& quat)
	{
		m_Position = pos;
		m_Quaternion = quat;
		m_Quaternion.Normalize();
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
		
		Matrix rotMatrix = Matrix::CreateFromQuaternion(m_Quaternion);
		//Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		forward = Vector3::Transform(forward, rotMatrix);

		Vector3 up{ 0, 1, 0 };
		up = Vector3::Transform(up, rotMatrix);

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
