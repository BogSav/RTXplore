#pragma once

#include "Camera.hpp"
#include "engine/math/Frustum.hpp"
#include "engine/core/Settings.hpp"

#include <array>

class CameraController
{
public:
	CameraController() = delete;
	CameraController(PerspectiveCamera& camera);

	void RefreshCamera();

	void InstantiateFrustum();

	// Rotation functions - first person
	void RotateFirstPerson_OX(float angle);
	void RotateFirstPerson_OY(float angle);

	// Rotation functions - third person
	void RotateThirdPerson_OX(float angle);
	void RotateThirdPerson_OY(float angle);

	// Translate functions
	void TranslateForward(float distance);
	void MoveForward(float distance);
	void TranslateUpward(float distance);
	void TranslateRight(float distance);

	// Update the camera and the world space frustrum
	void Update();

	const inline Math::Frustum& GetViewSpaceFrustum() const { return m_viewSpaceFrustum; }
	const inline Math::Frustum& GetWorldSpaceFrustum() const { return m_worldSpaceFrustum; }
	const inline PerspectiveCamera& GetCamera() const { return m_camera; }


	const inline bool IsDirty() const;
	inline void DecreaseDirtyCount();
	inline void SetDirty();

private:
	PerspectiveCamera& m_camera;

	UINT8 m_framesDirty;

	Math::Frustum m_viewSpaceFrustum;
	Math::Frustum m_worldSpaceFrustum;
};

class CubeMapCameraController
{
public:
	CubeMapCameraController();

	void Update(const Math::Vector3 pos);

	inline const Math::Frustum& GetFrustum(int index) const { return m_worldSpaceFrustums[index]; }
	inline const Math::Vector3 GetPosition() const { return m_cameras[0].GetPosition(); }
	inline const float GetZFar(int index) const { return m_cameras[index].GetZFar(); }
	inline const PerspectiveCamera& GetCamera(int index) const { return m_cameras[index]; }

private:
	std::array<PerspectiveCamera, 6> m_cameras;

	std::array<Math::Frustum, 6> m_viewSpaceFrustums;
	std::array<Math::Frustum, 6> m_worldSpaceFrustums;
};

inline const bool CameraController::IsDirty() const
{
	return m_framesDirty != 0;
}

inline void CameraController::DecreaseDirtyCount()
{
	assert(m_framesDirty > 0);

	m_framesDirty--;
}

inline void CameraController::SetDirty()
{
	m_framesDirty = Settings::GetFrameResourcesCount();
}