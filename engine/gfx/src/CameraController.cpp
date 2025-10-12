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
	m_camera.m_forward = engine::math::Quaternion(m_camera.m_right, engine::math::Scalar(angle)) * m_camera.m_forward;
	m_camera.m_forward.UnsafeNormalize();

	m_camera.m_up = engine::math::Quaternion(m_camera.m_right, engine::math::Scalar(angle)) * m_camera.m_up;
	m_camera.m_up.UnsafeNormalize();

	SetDirty();
}
void CameraController::RotateFirstPerson_OY(float angle)
{
	m_camera.m_right = engine::math::Quaternion(engine::math::Vector3{0, 1, 0},  engine::math::Scalar(angle)) * m_camera.m_right;
	m_camera.m_right.UnsafeNormalize();

	m_camera.m_forward = engine::math::Quaternion(engine::math::Vector3{0, 1, 0},  engine::math::Scalar(angle)) * m_camera.m_forward;
	m_camera.m_forward.UnsafeNormalize();

	m_camera.m_up = m_camera.m_right % m_camera.m_forward;
	m_camera.m_up.UnsafeNormalize();

	SetDirty();
}

// Rotation functions - third person
void CameraController::RotateThirdPerson_OX(float angle)
{
	engine::math::Matrix4 modelMatrix = engine::math::Matrix4(engine::math::kIdentity);

	modelMatrix = engine::math::Matrix4::MakeTranslation(m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = engine::math::Vector4(m_camera.m_position,  engine::math::Scalar(1)) * modelMatrix;

	RotateFirstPerson_OX(angle);

	modelMatrix = engine::math::Matrix4::MakeTranslation(-m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = engine::math::Vector4(m_camera.m_position,  engine::math::Scalar(1)) * modelMatrix;

	SetDirty();
}
void CameraController::RotateThirdPerson_OY(float angle)
{
	engine::math::Matrix4 modelMatrix = engine::math::Matrix4(engine::math::kIdentity);

	modelMatrix = engine::math::Matrix4::MakeTranslation(m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = engine::math::Vector4(m_camera.m_position,  engine::math::Scalar(1)) * modelMatrix;

	RotateFirstPerson_OY(angle);

	modelMatrix = engine::math::Matrix4::MakeTranslation(-m_camera.m_forward * m_camera.m_distanceToTarget);
	m_camera.m_position = engine::math::Vector4(m_camera.m_position,  engine::math::Scalar(1)) * modelMatrix;

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
	engine::math::Vector3 dir = (engine::math::Vector3(m_camera.m_forward.GetX(),  engine::math::Scalar(0), m_camera.m_forward.GetZ())).GetNormalized();
	m_camera.m_position += dir * distance;
	SetDirty();
}
void CameraController::TranslateUpward(float distance)
{
	m_camera.m_position += engine::math::Vector3{0, 1, 0} * distance;
	SetDirty();
}
void CameraController::TranslateRight(float distance)
{
	m_camera.m_position += (engine::math::Vector3{0, 1, 0} % (m_camera.m_right % engine::math::Vector3{0, 1, 0})) * distance;
	SetDirty();
}

void CameraController::Update()
{
	m_camera.m_viewMatrix = engine::math::Matrix4::MakeMatrixLookAtLH(
		m_camera.m_position, DirectX::XMVectorAdd(m_camera.m_position, m_camera.m_forward), m_camera.m_up);
	m_camera.m_invViewMatrix = engine::math::Matrix4::Inverse(m_camera.m_viewMatrix);

	m_worldSpaceFrustum = m_viewSpaceFrustum.GetTransformedFrustum(m_camera.m_invViewMatrix);
}

CubeMapCameraController::CubeMapCameraController()
{
	using namespace engine::math;

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

void CubeMapCameraController::Update(const engine::math::Vector3 center)
{
	for (int i = 0; i < 6; i++)
	{
		m_cameras[i].m_position = center;
		m_cameras[i].m_viewMatrix = engine::math::Matrix4::MakeMatrixLookAtLH(
			center, DirectX::XMVectorAdd(center, m_cameras[i].m_forward), m_cameras[i].m_up);
		m_cameras[i].m_invViewMatrix = engine::math::Matrix4::Inverse(m_cameras[i].m_viewMatrix);

		m_worldSpaceFrustums[i] = m_viewSpaceFrustums[i].GetTransformedFrustum(m_cameras[i].m_invViewMatrix);
	}
}
