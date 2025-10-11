#include "CameraController.hpp"

#include "Camera.hpp"

CameraController::CameraController(PerspectiveCamera& camera) : m_camera(camera)
{
	SetDirty();
}

void CameraController::RefreshCamera()
{
	m_camera.UpdateAspectRatio(Settings::GetGraphicsSettings().GetAspectRatio());
	m_viewSpaceFrustum = PerspectiveCamera::ConstructFrustum(m_camera);

	SetDirty();
}

void CameraController::InstantiateFrustum()
{
	m_viewSpaceFrustum = PerspectiveCamera::ConstructFrustum(m_camera);
	m_worldSpaceFrustum = m_viewSpaceFrustum.GetTransformedFrustum(m_camera.m_invViewMatrix);
}

// Rotation functions - first person
void CameraController::RotateFirstPerson_OX(float angle)
{
	m_camera.m_forward = Math::Quaternion(m_camera.m_right, Math::Scalar(angle)) * m_camera.m_forward;
	m_camera.m_forward.UnsafeNormalize();

	m_camera.m_up = Math::Quaternion(m_camera.m_right, Math::Scalar(angle)) * m_camera.m_up;
	m_camera.m_up.UnsafeNormalize();

	SetDirty();
}
void CameraController::RotateFirstPerson_OY(float angle)
{
	m_camera.m_right = Math::Quaternion(Math::Vector3{0, 1, 0},  Math::Scalar(angle)) * m_camera.m_right;
	m_camera.m_right.UnsafeNormalize();

	m_camera.m_forward = Math::Quaternion(Math::Vector3{0, 1, 0},  Math::Scalar(angle)) * m_camera.m_forward;
	m_camera.m_forward.UnsafeNormalize();

	m_camera.m_up = m_camera.m_right % m_camera.m_forward;
	m_camera.m_up.UnsafeNormalize();

	SetDirty();
}

// Rotation functions - third person
void CameraController::RotateThirdPerson_OX(float angle)
{
	Math::Matrix4 modelMatrix = Math::Matrix4(Math::kIdentity);

	modelMatrix = Math::Matrix4::MakeTranslation(m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = Math::Vector4(m_camera.m_position,  Math::Scalar(1)) * modelMatrix;

	RotateFirstPerson_OX(angle);

	modelMatrix = Math::Matrix4::MakeTranslation(-m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = Math::Vector4(m_camera.m_position,  Math::Scalar(1)) * modelMatrix;

	SetDirty();
}
void CameraController::RotateThirdPerson_OY(float angle)
{
	Math::Matrix4 modelMatrix = Math::Matrix4(Math::kIdentity);

	modelMatrix = Math::Matrix4::MakeTranslation(m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = Math::Vector4(m_camera.m_position,  Math::Scalar(1)) * modelMatrix;

	RotateFirstPerson_OY(angle);

	modelMatrix = Math::Matrix4::MakeTranslation(-m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = Math::Vector4(m_camera.m_position,  Math::Scalar(1)) * modelMatrix;

	SetDirty();
}

// Translate functions
void CameraController::TranslateForward(float distance)
{
	m_camera.m_position = m_camera.m_position + distance * m_camera.m_forward;
	SetDirty();
}
void CameraController::MoveForward(float distance)
{
	Math::Vector3 dir = (Math::Vector3(m_camera.m_forward.GetX(),  Math::Scalar(0), m_camera.m_forward.GetZ())).GetNormalized();
	m_camera.m_position += dir * distance;
	SetDirty();
}
void CameraController::TranslateUpward(float distance)
{
	m_camera.m_position += Math::Vector3{0, 1, 0} * distance;
	SetDirty();
}
void CameraController::TranslateRight(float distance)
{
	m_camera.m_position += (Math::Vector3{0, 1, 0} % (m_camera.m_right % Math::Vector3{0, 1, 0})) * distance;
	SetDirty();
}

void CameraController::Update()
{
	m_camera.m_viewMatrix = Math::Matrix4::MakeMatrixLookAtLH(
		m_camera.m_position, DirectX::XMVectorAdd(m_camera.m_position, m_camera.m_forward), m_camera.m_up);
	m_camera.m_invViewMatrix = Math::Matrix4::Inverse(m_camera.m_viewMatrix);

	m_worldSpaceFrustum = m_viewSpaceFrustum.GetTransformedFrustum(m_camera.m_invViewMatrix);
}

CubeMapCameraController::CubeMapCameraController()
{
	using namespace Math;

	// positive - x, negative - x, positive - y, negative - y, positive - z, negative - z.
	const Vector3 position = Vector3(kZero);
	const float& aspectRatio = 1.;

	m_cameras[0] = PerspectiveCamera(
		position, Vector3(1, 0, 0), Vector3(0, 1, 0), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);
	m_cameras[1] = PerspectiveCamera(
		position, Vector3(-1, 0, 0), Vector3(0, 1, 0), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);
	m_cameras[2] = PerspectiveCamera(
		position, Vector3(0, 1, 0), Vector3(0, 0, -1), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);
	m_cameras[3] = PerspectiveCamera(
		position, Vector3(0, -1, 0), Vector3(0, 0, 1), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);
	m_cameras[4] = PerspectiveCamera(
		position, Vector3(0, 0, 1), Vector3(0, 1, 0), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);
	m_cameras[5] = PerspectiveCamera(
		position, Vector3(0, 0, -1), Vector3(0, 1, 0), aspectRatio, DirectX::XMConvertToRadians(90.f), 150.f);

	for (int i = 0; i < 6; i++)
	{
		m_viewSpaceFrustums[i] = PerspectiveCamera::ConstructFrustum(m_cameras[i]);
		m_worldSpaceFrustums[i] = m_viewSpaceFrustums[i].GetTransformedFrustum(m_cameras[i].m_viewMatrix);
	}
}

void CubeMapCameraController::Update(const Math::Vector3 center)
{
	for (int i = 0; i < 6; i++)
	{
		m_cameras[i].m_position = center;
		m_cameras[i].m_viewMatrix = Math::Matrix4::MakeMatrixLookAtLH(
			center, DirectX::XMVectorAdd(center, m_cameras[i].m_forward), m_cameras[i].m_up);
		m_cameras[i].m_invViewMatrix = Math::Matrix4::Inverse(m_cameras[i].m_viewMatrix);

		m_worldSpaceFrustums[i] = m_viewSpaceFrustums[i].GetTransformedFrustum(m_cameras[i].m_invViewMatrix);
	}
}
