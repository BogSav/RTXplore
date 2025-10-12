#pragma once

#include "Matrix4.hpp"
#include "VectorUtils.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace engine::math
{

class AABB
{
public:
	INLINE AABB();
	INLINE void SetCorners(const Vector3& corner1, const Vector3& corner2);
	INLINE AABB(const Vector3& corner1, const Vector3& corner2, bool checkCorners = true);
	INLINE AABB(const Vector3& bboxCenter, const Scalar& cubeSize);

	INLINE bool operator==(const AABB& toCompare) const;
	INLINE bool operator!=(const AABB& toCompare) const;
	INLINE AABB& operator+=(const AABB& boxToAdd);
	INLINE AABB operator+(const AABB& boxToAdd) const;
	void INLINE operator=(const AABB& other);

	// Getteri basic
	INLINE Vector3 GetCenter() const;

	INLINE Scalar GetMinX() const { return GetMin().GetX(); }
	INLINE Scalar GetMinY() const { return GetMin().GetY(); }
	INLINE Scalar GetMinZ() const { return GetMin().GetZ(); }

	INLINE const Vector3& GetMin() const { return m_minCorner; }
	INLINE const Vector3& GetMax() const { return m_maxCorner; }

	INLINE Scalar GetMaxX() const { return GetMax().GetX(); }
	INLINE Scalar GetMaxY() const { return GetMax().GetY(); }
	INLINE Scalar GetMaxZ() const { return GetMax().GetZ(); }

	INLINE Scalar GetSizeX() const { return GetSize().GetX(); }
	INLINE Scalar GetSizeY() const { return GetSize().GetY(); }
	INLINE Scalar GetSizeZ() const { return GetSize().GetZ(); }

	// Getteri proprietati bbox
	INLINE Vector3 GetSize() const;
	INLINE Scalar GetVolume() const;
	INLINE Scalar GetSurfaceArea() const;
	INLINE Scalar GetDiagonalLength() const { return ~GetSize(); }

	// Checkeri proprietati bbox
	INLINE bool IsSinglePoint() const;
	INLINE bool IsPlane() const;

	// Tranformari pentru bbox
	AABB& Transform(const Matrix4& m);
	AABB Transformed(const Matrix4& m) const { return AABB(*this).Transform(m); }

	INLINE AABB& EnlargeForPoint(const Vector3& point);
	INLINE AABB& EnlargeForPoints(std::initializer_list<Vector3> v);

	INLINE AABB& Inflate(const Scalar dx, const Scalar dy, const Scalar dz);
	AABB Inflated(const Scalar dx, const Scalar dy, const Scalar dz) const { return AABB(*this).Inflate(dx, dy, dz); }
	INLINE AABB& Inflate(const Scalar d) { return Inflate(d, d, d); }
	AABB Inflated(const Scalar d) const { return Inflated(d, d, d); }

	// Validatori
	INLINE bool IsInitialized() const;
	INLINE void Uninitialize();

	// Checkeri
	INLINE bool Contains(const Vector3& p) const;
	INLINE void Intersect(const AABB& other);
	INLINE bool IsIntersecting(const AABB& other) const;

	std::string ToString() const;

	class Iterator
	{
	public:
		INLINE Iterator(const AABB* bb, int idx);
		INLINE Iterator();

		INLINE Iterator& operator++();
		INLINE bool operator!=(const Iterator& i) const;

	private:
		const AABB* m_bb;
		int m_idx;
	};  // end of class Iterator

	INLINE Iterator Begin() const;
	INLINE Iterator End() const;

private:
	Vector3 m_minCorner;
	Vector3 m_maxCorner;

	INLINE void ValidateIfBoxIsInitialized() const;
};


AABB::AABB(const Vector3& bboxCenter, const Scalar& cubeSize) : m_minCorner(bboxCenter), m_maxCorner(bboxCenter)
{
	Inflate(cubeSize * static_cast<Scalar>(0.5));
}


INLINE void AABB::ValidateIfBoxIsInitialized() const
{
	if (!IsInitialized())
		throw std::runtime_error("Uninitialized bounding box used");
}


INLINE typename AABB::Iterator AABB::End() const
{
	return Iterator(this, 8);
}


INLINE typename AABB::Iterator AABB::Begin() const
{
	return Iterator(this, 0);
}


INLINE AABB& AABB::Inflate(const Scalar dx, const Scalar dy, const Scalar dz)
{
	if (!IsInitialized())
	{
		return *this;
	}

	const Vector3 offs(dx, dy, dz);
	m_minCorner -= offs;
	m_maxCorner += offs;
	const Vector3 size = m_maxCorner - m_minCorner;
	if (static_cast<float>(size.GetX()) < 0.f || 
	    static_cast<float>(size.GetY()) < 0.f || 
	    static_cast<float>(size.GetZ()) < 0.f)
	{
		Uninitialize();
	}
	return *this;
}


INLINE bool AABB::Contains(const Vector3& p) const
{
	return p == GetMaxPoint(p, m_minCorner) && p == GetMinPoint(p, m_maxCorner);
}


INLINE void AABB::Uninitialize()
{
	m_maxCorner = Vector3(-maxvalue(1.0f), -maxvalue(1.0f), -maxvalue(1.0f));
	m_minCorner = Vector3(maxvalue(1.0f), maxvalue(1.0f), maxvalue(1.0f));
}
//-----------------------------------------------------------------

INLINE bool AABB::IsInitialized() const
{
	constexpr float maxVal = maxvalue(1.0f);
	return !(
		static_cast<float>(m_maxCorner.GetX()) == -maxVal && 
		static_cast<float>(m_maxCorner.GetY()) == -maxVal &&
		static_cast<float>(m_maxCorner.GetZ()) == -maxVal && 
		static_cast<float>(m_minCorner.GetX()) == maxVal &&
		static_cast<float>(m_minCorner.GetY()) == maxVal && 
		static_cast<float>(m_minCorner.GetZ()) == maxVal);
}


INLINE AABB& AABB::EnlargeForPoint(const Vector3& point)
{
	m_minCorner = GetMinPoint(m_minCorner, point);
	m_maxCorner = GetMaxPoint(m_maxCorner, point);
	return *this;
}

INLINE AABB& AABB::EnlargeForPoints(typename std::initializer_list<Vector3> v)
{
	std::for_each(v.begin(), v.end(), [this](const auto& p) { this->EnlargeForPoint(p); });
	return *this;
}


INLINE Vector3 AABB::GetCenter() const
{
	ValidateIfBoxIsInitialized();
	return (m_minCorner + m_maxCorner) * static_cast<Scalar>(0.5);
}


INLINE bool AABB::IsPlane() const
{
	return IsInitialized() && (float(GetSizeX()) == 0.0f || float(GetSizeY()) == 0.0f || float(GetSizeZ()) == 0.0f);
}


INLINE bool AABB::IsSinglePoint() const
{
	return m_minCorner == m_maxCorner;
}


INLINE void AABB::Intersect(const AABB& other)
{
	Vector3 newMaxCorner = GetMinPoint(m_maxCorner, other.m_maxCorner);
	Vector3 newMinCorner = GetMaxPoint(m_minCorner, other.m_minCorner);

	// need to check if intersection is empty
	if (GetMinPoint(newMaxCorner, newMinCorner) == newMinCorner)
	{
		m_maxCorner = newMaxCorner;
		m_minCorner = newMinCorner;
	}
	else
	{
		Uninitialize();
	}
}


INLINE bool AABB::IsIntersecting(const AABB& other) const
{
	return !(
		static_cast<float>(m_maxCorner.GetX()) < static_cast<float>(other.m_minCorner.GetX()) || 
		static_cast<float>(m_maxCorner.GetY()) < static_cast<float>(other.m_minCorner.GetY()) ||
		static_cast<float>(m_maxCorner.GetZ()) < static_cast<float>(other.m_minCorner.GetZ()) || 
		static_cast<float>(m_minCorner.GetX()) > static_cast<float>(other.m_maxCorner.GetX()) ||
		static_cast<float>(m_minCorner.GetY()) > static_cast<float>(other.m_maxCorner.GetY()) || 
		static_cast<float>(m_minCorner.GetZ()) > static_cast<float>(other.m_maxCorner.GetZ()));
}


INLINE void AABB::operator=(const AABB& other)
{
	//Vector3 newMaxCorner = GetMaxPoint(m_maxCorner, other.m_maxCorner);
	//Vector3 newMinCorner = GetMinPoint(m_minCorner, other.m_minCorner);
	//// need to check if intersection is empty
	//if (GetMinPoint(newMaxCorner, newMinCorner) == newMinCorner)
	//{
		m_maxCorner = other.GetMax();
		m_minCorner = other.GetMin();
	//}
	//else
	//{
	//	m_maxCorner = m_minCorner;
	//}
}


INLINE AABB& AABB::operator+=(const AABB& boxToAdd)
{
	m_maxCorner = GetMaxPoint(m_maxCorner, boxToAdd.m_maxCorner);
	m_minCorner = GetMinPoint(m_minCorner, boxToAdd.m_minCorner);
	return *this;
}


INLINE AABB AABB::operator+(const AABB& boxToAdd) const
{
	AABB ret = *this;
	ret += boxToAdd;
	return ret;
}


INLINE Scalar AABB::GetSurfaceArea() const
{
	const Vector3& size = GetSize();
	return (size.GetX() * size.GetY() + size.GetY() * size.GetZ() + size.GetX() * size.GetZ()) * Scalar(2);
}


inline Vector3 AABB::GetSize() const
{
	return m_maxCorner - m_minCorner;
}

INLINE Scalar AABB::GetVolume() const
{
	const Vector3& size = GetSize();
	return size.GetX() * size.GetY() * size.GetZ();
}


INLINE bool AABB::operator!=(const AABB& toCompare) const
{
	return !(*this == toCompare);
}


INLINE bool AABB::operator==(const AABB& toCompare) const
{
	return m_minCorner == toCompare.m_minCorner && m_maxCorner == toCompare.m_maxCorner;
}


INLINE AABB::AABB(const Vector3& corner1, const Vector3& corner2, bool checkCorners)
{
	if (checkCorners)
	{
		SetCorners(corner1, corner2);
	}
	else
	{
		m_minCorner = corner1;
		m_maxCorner = corner2;
	}
}


INLINE void AABB::SetCorners(const Vector3& corner1, const Vector3& corner2)
{
	m_maxCorner = GetMaxPoint(corner1, corner2);
	m_minCorner = GetMinPoint(corner1, corner2);
}


INLINE AABB::AABB()
	: m_minCorner(maxvalue(1.0f), maxvalue(1.0f), maxvalue(1.0f)),
	  m_maxCorner(-maxvalue(1.0f), -maxvalue(1.0f), -maxvalue(1.0f))
{
}

// ADL for begin/end (allows range-based for loop for bounding box corners)

INLINE typename AABB::Iterator begin(const AABB& bbox)
{
	return bbox.Begin();
}

INLINE typename AABB::Iterator end(const AABB& bbox)
{
	return bbox.End();
}
//------------------------------------------------------------------------------

INLINE bool AABB::Iterator::operator!=(const Iterator& i) const
{
	return m_idx != i.m_idx;
}
//------------------------------------------------------------------------------

INLINE typename AABB::Iterator& AABB::Iterator::operator++()
{
	++m_idx;
	return *this;
}
//------------------------------------------------------------------------------

INLINE AABB::Iterator::Iterator() : m_bb(NULL), m_idx(0)
{
}
//------------------------------------------------------------------------------

INLINE AABB::Iterator::Iterator(const AABB* bb, int idx) : m_bb(bb), m_idx(idx)
{
}

}  // namespace engine::math
