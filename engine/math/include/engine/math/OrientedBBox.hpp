#pragma once

#include "AxisAllignedBBox.hpp"

namespace Math
{
class OBB
{
public:
	OBB() : m_center(Vector3(0.f, 0.f, 0.f)), m_extension(Vector3(0.f, 0.f, 0.f))
	{
		m_axis[0] = Vector3(1.f, 0.f, 0.f);
		m_axis[1] = Vector3(0.f, 1.f, 0.f);
		m_axis[2] = Vector3(0.f, 0.f, 1.f);
	}

	OBB(const AABB& aabb)
		: m_center(aabb.GetCenter()),
		  m_extension(Vector3(
			  (aabb.GetMaxX() - aabb.GetMinX()) * 0.5f,
			  (aabb.GetMaxY() - aabb.GetMinY()) * 0.5f,
			  (aabb.GetMaxZ() - aabb.GetMinZ()) * 0.5f))
	{
		m_axis[0] = Vector3(1.f, 0.f, 0.f);
		m_axis[1] = Vector3(0.f, 1.f, 0.f);
		m_axis[2] = Vector3(0.f, 0.f, 1.f);
	}

	OBB(const AABB& aabb, const Matrix4& matrix)
		: m_center(aabb.GetCenter()),
		  m_extension(Vector3(
			  (aabb.GetMaxX() - aabb.GetMinX()) * 0.5f,
			  (aabb.GetMaxY() - aabb.GetMinY()) * 0.5f,
			  (aabb.GetMaxZ() - aabb.GetMinZ()) * 0.5f))
	{
		m_center = m_center * matrix;
		m_axis[0] = Vector3(matrix[0][0], matrix[1][0], matrix[2][0]);
		m_axis[1] = Vector3(matrix[0][1], matrix[1][1], matrix[2][1]);
		m_axis[2] = Vector3(matrix[0][2], matrix[1][2], matrix[2][2]);
	}

	inline Scalar GetDiagonalLength() const { return ~(GetMinCorner() - GetMaxCorner()); }

	inline Vector3 GetMinCorner() const
	{
		return m_center - m_extension[0] * m_axis[0] - m_extension[1] * m_axis[1] - m_extension[2] * m_axis[2];
	}

	inline Vector3 GetMaxCorner() const
	{
		return m_center + m_extension[0] * m_axis[0] + m_extension[1] * m_axis[1] + m_extension[2] * m_axis[2];
	}

	bool IsPointInsideOBB(const Vector3& point)
	{
		Vector3 localPoint = point - m_center;

		for (int i = 0; i < 3; i++)
		{
			Scalar dist = localPoint * m_axis[i];

			if ((float)dist > (float)m_extension[i] || (float)dist < -(float)m_extension[i])
				return false;
		}

		return true;
	};

public:
	Vector3 m_center;
	Vector3 m_axis[3];
	Vector3 m_extension;
};

}  // namespace Math