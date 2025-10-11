#pragma once

#include "engine/math/OrientedBBox.hpp"
#include "engine/math/Quaternion.hpp"
#include "engine/math/Frustum.hpp"

#include <memory>

class BaseCamera
{
public:
	using Ptr = std::shared_ptr<BaseCamera>;

	BaseCamera() = default;
	BaseCamera(
		const Math::Vector3 position,
		const Math::Vector3 center,
		const Math::Vector3 up,
		const float zFar,
		const float zNear)
		: m_zFar(zFar), m_zNear(zNear)
	{
		SetViewCoordinateSystem(position, center, up);
	};

	void SetViewCoordinateSystem(const Math::Vector3 position, const Math::Vector3 center, const Math::Vector3 up)
	{
		m_position = position;
		m_forward = (center - position).GetNormalized();
		m_right = (m_forward % up).GetNormalized();
		m_up = (m_right % m_forward).GetNormalized();
		m_distanceToTarget = ~(center - position);

		ConstructViewMatrix();
	}

	void SetViewCoordinateSystem(const Math::Vector3 position, const Math::Vector3 center)
	{
		m_position = position;
		m_distanceToTarget = ~(center - position);
		SetViewCoordinateSystem((center - position).GetNormalized());
	}

	void SetViewCoordinateSystem(const Math::Vector3 direction)
	{
		m_forward = direction;

		m_up = Math::Vector3(0, 1, 0);
		m_right = (m_up % m_forward).GetNormalized();

		m_up = (m_forward % m_right).GetNormalized();

		ConstructViewMatrix();
	}

	inline Math::Matrix4 GetViewProjMatrix() const { return GetViewMatrix() * GetProjectionMatrix(); }

	// Getters for components
	inline const Math::Vector3& GetTargetPosition() const { return m_position + m_forward * m_distanceToTarget; }
	inline const Math::Matrix4& GetProjectionMatrix() const { return m_projectionMatrix; }
	inline const Math::Matrix4& GetInverseViewMatrix() const { return m_invViewMatrix; }
	inline const Math::Matrix4& GetViewMatrix() const { return m_viewMatrix; }
	inline const Math::Vector3& GetPosition() const { return m_position; };
	inline const Math::Vector3& GetForward() const { return m_forward; }
	inline const Math::Vector3 GetUp() const { return m_up; }

	inline float GetZNear() const { return m_zNear; }
	inline float GetZFar() const { return m_zFar; }

	friend class CameraController;
	friend class CubeMapCameraController;

private:
	void ConstructViewMatrix()
	{
		// Matricea de view
		m_viewMatrix = Math::Matrix4::MakeMatrixLookAtLH(m_position, DirectX::XMVectorAdd(m_position, m_forward), m_up);
		m_invViewMatrix = Math::Matrix4::Inverse(m_viewMatrix);
	}
	virtual void ConstructProjectionMatrix() = 0;

protected:
	Math::Matrix4 m_projectionMatrix = Math::Matrix4();
	Math::Matrix4 m_viewMatrix = Math::Matrix4();
	Math::Matrix4 m_invViewMatrix = Math::Matrix4();

	Math::Vector3 m_position = Math::Vector3();

	Math::Vector3 m_forward = Math::Vector3();
	Math::Vector3 m_right = Math::Vector3();
	Math::Vector3 m_up = Math::Vector3();

	Math::Scalar m_distanceToTarget = {};

	float m_zNear = 0.f;
	float m_zFar = 0.f;
};

class PerspectiveCamera : public BaseCamera
{
public:
	using Ptr = std::shared_ptr<PerspectiveCamera>;

	PerspectiveCamera() = default;
	PerspectiveCamera(
		const Math::Vector3 position,
		const Math::Vector3 center,
		const Math::Vector3 up,
		const float aspectRatio,
		const float fovY = DirectX::XMConvertToRadians(60.f),
		const float zFar = 250.f,
		const float zNear = 0.5f)
		: BaseCamera(position, center, up, zFar, zNear)
	{
		m_aspectRatio = aspectRatio;

		// FoV - verical field of view
		m_fovY = fovY;

		// Set near and far windows heights
		const float tanHalfFovY = tanf(fovY / 2.f);

		m_farWindowHeight = 2.f * tanHalfFovY * m_zFar;
		m_nearWindowHeight = 2.f * tanHalfFovY * m_zNear;

		// FoH - horizontal field of view
		m_fovX = 2.f * atanf(GetNearWindowWidth() / (2.0f * m_zNear));

		ConstructProjectionMatrix();
	}

	static Math::Frustum ConstructFrustum(const PerspectiveCamera& camera)
	{
		Math::Frustum frustrum;
		const float farHalfWidth = camera.GetFarWindowWidth() / 2.f;
		const float nearHalfWidth = camera.GetNearWindowWidth() / 2.f;

		const float farHalfHeight = camera.GetFarWindowHeight() / 2.f;
		const float nearHalfHeight = camera.GetNearWindowHeight() / 2.f;

		// 0 - Near Jos Dreapta
		// 1 - Near Jos Stanga
		// 2 - Near Sus Stanga
		// 3 - Near Sus Dreapta
		frustrum.GetCorner(0) = Math::Point3(nearHalfWidth, -nearHalfHeight, camera.GetZNear());
		frustrum.GetCorner(1) = Math::Point3(-nearHalfWidth, -nearHalfHeight, camera.GetZNear());
		frustrum.GetCorner(2) = Math::Point3(-nearHalfWidth, nearHalfHeight, camera.GetZNear());
		frustrum.GetCorner(3) = Math::Point3(nearHalfWidth, nearHalfHeight, camera.GetZNear());

		// 4 - Far Jos Dreapta
		// 5 - Far Jos Stanga
		// 6 - Far Sus Stanga
		// 7 - Far Sus Dreapta
		frustrum.GetCorner(4) = Math::Point3(farHalfWidth, -farHalfHeight, camera.GetZFar());
		frustrum.GetCorner(5) = Math::Point3(farHalfWidth, -farHalfHeight, camera.GetZFar());
		frustrum.GetCorner(6) = Math::Point3(farHalfWidth, -farHalfHeight, camera.GetZFar());
		frustrum.GetCorner(7) = Math::Point3(farHalfWidth, -farHalfHeight, camera.GetZFar());

		// 0 - Near plane
		// 1 - Far plane
		frustrum.GetBoundingPlane(0) =
			Math::BoundingPlane(Math::Vector3(0, 0, 1), Math::Point3(0, 0, camera.GetZNear()));
		frustrum.GetBoundingPlane(1) =
			Math::BoundingPlane(Math::Vector3(0, 0, -1), Math::Point3(0, 0, camera.GetZFar()));

		// 2 - Right plane
		// 3 - Left plane
		frustrum.GetBoundingPlane(2) =
			Math::BoundingPlane(Math::Vector3(-camera.GetZFar(), 0, farHalfWidth), Math::Scalar(0));
		frustrum.GetBoundingPlane(3) =
			Math::BoundingPlane(Math::Vector3(camera.GetZFar(), 0, farHalfWidth), Math::Scalar(0));

		// 4 - Upper plane
		// 5 - Bottom plane
		frustrum.GetBoundingPlane(4) =
			Math::BoundingPlane(Math::Vector3(0, -camera.GetZFar(), farHalfHeight), Math::Scalar(0));
		frustrum.GetBoundingPlane(5) =
			Math::BoundingPlane(Math::Vector3(0, camera.GetZFar(), farHalfHeight), Math::Scalar(0));

		return frustrum;
	}

	void UpdateAspectRatio(float aspectRatio)
	{
		m_aspectRatio = aspectRatio;
		m_projectionMatrix = Math::Matrix4::MakeMatrixPerspectiveFovLH(m_fovY, aspectRatio, m_zNear, m_zFar);
		m_fovX = 2.f * atanf(GetNearWindowWidth() / (2.0f * m_zNear));
	}

	inline float GetFovY() const { return m_fovY; }
	inline float GetFovX() const { return m_fovY; }

	inline float GetNearWindowWidth() const { return m_aspectRatio * m_nearWindowHeight; }
	inline float GetFarWindowWidth() const { return m_aspectRatio * m_farWindowHeight; }
	inline float GetNearWindowHeight() const { return m_nearWindowHeight; }
	inline float GetFarWindowHeight() const { return m_farWindowHeight; }

private:
	void ConstructProjectionMatrix() override
	{
		m_projectionMatrix = Math::Matrix4::MakeMatrixPerspectiveFovLH(m_fovY, m_aspectRatio, m_zNear, m_zFar);
	}

private:
	float m_fovY = 0.f;
	float m_fovX = 0.f;

	float m_nearWindowHeight = 0.f;
	float m_farWindowHeight = 0.f;

	float m_aspectRatio = 0.f;
};

class OrthograficCamera : public BaseCamera
{
public:
	using Ptr = std::shared_ptr<OrthograficCamera>;

	OrthograficCamera() = default;
	OrthograficCamera(
		const Math::Vector3 position,
		const Math::Vector3 center,
		const Math::Vector3 up,
		const float leftOrtho = -100.f,
		const float rightOrtho = 100.f,
		const float bottomOrtho = -100.f,
		const float topOrtho = 100.f,
		const float zFar = 150.f,
		const float zNear = 0.5f)
		: BaseCamera(position, center, up, zFar, zNear),
		  m_leftOrtho(leftOrtho),
		  m_rightOrtho(rightOrtho),
		  m_bottomOrtho(bottomOrtho),
		  m_topOrtho(topOrtho)
	{
		ConstructProjectionMatrix();
	}

private:
	void ConstructProjectionMatrix() override
	{
		m_projectionMatrix = Math::Matrix4::MakeMatrixOrthographicOffCenterLH(
			m_leftOrtho, m_rightOrtho, m_bottomOrtho, m_topOrtho, m_zNear, m_zFar);
	}

private:
	float m_leftOrtho = -50.f;
	float m_rightOrtho = 50.f;
	float m_bottomOrtho = -50.f;
	float m_topOrtho = 50.f;
};