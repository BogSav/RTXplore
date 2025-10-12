#pragma once

#include "Vector.hpp"

#include <cmath>

namespace engine::math
{

class BoundingPlane
{
public:
	// Constructorii asigura ca normala este normalizata
	BoundingPlane() = default;
	BoundingPlane(Vector3 normalToPlane, Scalar distance)
	{
		planeData = Vector4(normalToPlane.GetNormalized(), distance);
	}
	BoundingPlane(Vector3 normalToPlane, Point3 pointOnPlane)
	{
		Vector3 normalizedNormal = normalToPlane.GetNormalized();

		planeData = Vector4(normalizedNormal, -(normalizedNormal * pointOnPlane));
	}

	Vector3 GetNormal() const { return Vector3(planeData); }
	Scalar GetDistanceFromOrigin() const { return planeData.GetW(); }

	Scalar DistanceFromPoint(Vector3 point) const { return (point * GetNormal()) + planeData.GetW(); }
	Scalar DistanceFromPoint(Vector4 point) const { return point * planeData; }

	BoundingPlane TransformToWorldSpace(const Matrix4& invViewMatrix) const
	{
		return BoundingPlane(
			invViewMatrix.TransformNormal(this->GetNormal()),
			invViewMatrix.TransformPoint(this->GetNormal() * -this->GetDistanceFromOrigin()));
	}

	Vector3 IntersectWithRay(Vector3 rayOrigin, Vector3 rayDirection) const
	{
		Vector3 normal = GetNormal();
		Scalar denominator = normal * rayDirection;

		if (std::abs((float)denominator) < std::numeric_limits<float>::epsilon())
		{
			return Vector3(kZero);
		}

		Scalar distance = GetDistanceFromOrigin();
		Scalar t = -(rayOrigin * normal + distance) / denominator;

		if ((float)t < 0.f)
		{
			return Vector3(kZero);
		}

		return rayOrigin + rayDirection * t;
	}


private:
	// (x,y,z,w) unde (x,y,z) este normala la plan si w este distanta fata de origine
	Vector4 planeData;
};

}  // namespace engine::math