#pragma once

#include "AxisAllignedBBox.hpp"
#include "BoundingPlane.h"

namespace engine::math
{
class Frustum
{
public:
	Frustum() = default;

	bool IntersectBoundingBox(const AABB& aabb) const;
	Frustum GetTransformedFrustum(const Matrix4& viewMatrix) const;

	BoundingPlane& GetBoundingPlane(size_t index);
	engine::math::Point3& GetCorner(size_t index);

private:
	BoundingPlane m_frustumPlanes[6];
	engine::math::Point3 m_frustumCorners[8];
};

}  // namespace engine::math
