#include "Frustum.hpp"

using namespace engine::math;

bool engine::math::Frustum::IntersectBoundingBox(const AABB& aabb) const
{
	for (int i = 0; i < 6; ++i)
	{
		BoundingPlane p = m_frustumPlanes[i];
		Vector3 farCorner = Vector3::Select(aabb.GetMin(), aabb.GetMax(), p.GetNormal() > Vector3(kZero));
		if ((float)p.DistanceFromPoint(farCorner) < 0.0f)
		{
			return false;
		}
	}

	return true;
}

// Se fac transformarile din view space in world space ale planelor si ale pucntelor de frustum
Frustum engine::math::Frustum::GetTransformedFrustum(const Matrix4& invViewMatrix) const
{
	Frustum result;

	for (int i = 0; i < 6; ++i)
	{
		result.m_frustumPlanes[i] = this->m_frustumPlanes[i].TransformToWorldSpace(invViewMatrix);
	}

	for (int i = 0; i < 8; ++i)
	{
		result.m_frustumCorners[i] = this->m_frustumCorners[i] * invViewMatrix;
	}

	return result;
}

BoundingPlane& Frustum::GetBoundingPlane(size_t index)
{
	return m_frustumPlanes[index];
}

engine::math::Point3& Frustum::GetCorner(size_t index)
{
	return m_frustumCorners[index];
}