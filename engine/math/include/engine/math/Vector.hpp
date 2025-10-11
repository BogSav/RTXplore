#pragma once

#include "Common.hpp"
#include "Scalar.hpp"

#include <string>
#include <stdexcept>

namespace Math
{
class Vector4;

__declspec(align(16)) class Vector3
{
public:
#pragma warning(push)
#pragma warning(disable : 26495)
	INLINE Vector3() {}
#pragma warning(pop)

	INLINE Vector3(float x, float y, float z) { m_vec = XMVectorSet(x, y, z, z); }
	INLINE Vector3(Scalar x, Scalar y, Scalar z) { m_vec = XMVectorSet(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(z)); }
	INLINE Vector3(const XMFLOAT3& v) { m_vec = XMLoadFloat3(&v); }
	INLINE Vector3(const Vector3& v) { m_vec = v; }
	INLINE Vector3(Scalar s) { m_vec = s; }
	INLINE explicit Vector3(Vector4 vec);
	INLINE explicit Vector3(FXMVECTOR vec) { m_vec = vec; }
	INLINE explicit Vector3(EZeroTag) { m_vec = SplatZero(); }
	INLINE explicit Vector3(EIdentityTag) { m_vec = SplatOne(); }
	INLINE explicit Vector3(EXUnitVector) { m_vec = CreateXUnitVector(); }
	INLINE explicit Vector3(EYUnitVector) { m_vec = CreateYUnitVector(); }
	INLINE explicit Vector3(EZUnitVector) { m_vec = CreateZUnitVector(); }

	INLINE Vector3& operator=(const Vector3& v) { m_vec = v; return *this; }

	INLINE operator XMVECTOR() const { return m_vec; }
	INLINE operator XMFLOAT3() const
	{
		XMFLOAT3 v;
		XMStoreFloat3(&v, m_vec);
		return v;
	}

	INLINE Scalar GetX() const { return Scalar(XMVectorSplatX(m_vec)); }
	INLINE Scalar GetY() const { return Scalar(XMVectorSplatY(m_vec)); }
	INLINE Scalar GetZ() const { return Scalar(XMVectorSplatZ(m_vec)); }
	INLINE void SetX(Scalar x) { m_vec = XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
	INLINE void SetY(Scalar y) { m_vec = XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
	INLINE void SetZ(Scalar z) { m_vec = XMVectorPermute<0, 1, 6, 3>(m_vec, z); }

	INLINE void operator=(Vector4 vec);
	INLINE Vector3 operator-() const { return Vector3(XMVectorNegate(m_vec)); }
	INLINE Scalar operator~() const { return Scalar(XMVector3Length(m_vec)); }
	INLINE Scalar operator[](int pos) const
	{
		if (pos == 0)
			return GetX();
		else if (pos == 1)
			return GetY();
		else if (pos == 2)
			return GetZ();
		else
			throw std::runtime_error("Indexul a depasit dimensiunea vectorului: index - " + std::to_string(pos));
	}
	INLINE Vector3 operator>(Vector3 v2) const { return Vector3(XMVectorGreater(m_vec, v2)); }
	INLINE Vector3 operator>=(Vector3 v2) const { return Vector3(XMVectorGreaterOrEqual(m_vec, v2)); }

	INLINE Vector3 operator+(Vector3 v2) const { return Vector3(XMVectorAdd(m_vec, v2)); }
	INLINE Vector3 operator-(Vector3 v2) const { return Vector3(XMVectorSubtract(m_vec, v2)); }
	INLINE Scalar operator*(Vector3 v2) const { return Scalar(XMVector3Dot(m_vec, v2)); }
	INLINE Vector3 operator/(Vector3 v2) const { return Vector3(XMVectorDivide(m_vec, v2)); }
	INLINE Vector3 operator%(Vector3 v2) const { return Vector3(XMVector3Cross(m_vec, v2)); }
	INLINE Vector3 operator*(Scalar v2) const { return Vector3(XMVectorMultiply(m_vec, Vector3(v2))); }
	INLINE Vector3 operator/(Scalar v2) const { return *this / Vector3(v2); }
	INLINE Vector3 operator*(float v2) const { return *this * Scalar(v2); }
	INLINE Vector3 operator/(float v2) const { return *this / Scalar(v2); }

	INLINE Vector3& operator+=(Vector3 v)
	{
		*this = *this + v;
		return *this;
	}
	INLINE Vector3& operator-=(Vector3 v)
	{
		*this = *this - v;
		return *this;
	}
	INLINE Vector3& operator*=(Vector3 v)
	{
		*this = Vector3(XMVectorMultiply(m_vec, v));
		return *this;
	}
	INLINE Vector3& operator/=(Vector3 v)
	{
		*this = *this / v;
		return *this;
	}
	INLINE Vector3& operator%=(Vector3 v)
	{
		*this = *this % v;
		return *this;
	}

	INLINE friend Vector3 operator*(Scalar v1, Vector3 v2) { return Vector3(XMVectorMultiply(Vector3(v1), v2)); }
	INLINE friend Vector3 operator/(Scalar v1, Vector3 v2) { return Vector3(v1) / v2; }
	INLINE friend Vector3 operator*(float v1, Vector3 v2) { return Scalar(v1) * v2; }
	INLINE friend Vector3 operator/(float v1, Vector3 v2) { return Scalar(v1) / v2; }
	INLINE friend bool operator==(Vector3 lhs, Vector3 rhs)
	{
		return IsNear(static_cast<float>(lhs.GetX()), static_cast<float>(rhs.GetX())) && 
		       IsNear(static_cast<float>(lhs.GetY()), static_cast<float>(rhs.GetY())) && 
		       IsNear(static_cast<float>(lhs.GetZ()), static_cast<float>(rhs.GetZ()));
	}

	INLINE void Normalize()
	{
		if (IsNearZero(static_cast<float>(GetX())) && 
		    IsNearZero(static_cast<float>(GetY())) && 
		    IsNearZero(static_cast<float>(GetZ())))
			throw std::runtime_error("Cannot normalize null vector");

		UnsafeNormalize();
	}
	INLINE void NormalizeEst()
	{
		if (IsNearZero(static_cast<float>(GetX())) && 
		    IsNearZero(static_cast<float>(GetY())) && 
		    IsNearZero(static_cast<float>(GetZ())))
			throw std::runtime_error("Cannot normalize null vector");

		UnsafeNormalizeEst();
	}
	INLINE void UnsafeNormalize() { m_vec = XMVector3Normalize(m_vec); };
	INLINE void UnsafeNormalizeEst() { m_vec = XMVector3NormalizeEst(m_vec); }
	INLINE Vector3 GetNormalized() const { return Vector3(XMVector3Normalize(m_vec)); }
	INLINE Vector3 GetNormalizedEst() const { return Vector3(XMVector3NormalizeEst(m_vec)); }
	INLINE float Length2() const { return XMVectorGetX(XMVector3LengthSq(m_vec)); }

	INLINE std::string ToString() const
	{
		//return "X: " + std::to_string(float(GetX())) + " - Y: " + std::to_string(float(GetY()))
		//	+ " - Z: " + std::to_string(float(GetZ()));
		return "( " + std::to_string(float(GetX())) + " , " + std::to_string(float(GetY())) + " , "
			+ std::to_string(float(GetZ())) + ")";
	}

	static INLINE Vector3 Select(const Vector3& lhs, const Vector3& rhs, const Vector3& mask)
	{
		return Vector3(XMVectorSelect(lhs, rhs, mask));
	}

protected:
	XMVECTOR m_vec;
};

// A 4-vector, completely defined.
class Vector4
{
public:
#pragma warning(push)
#pragma warning(disable : 26495)
	INLINE Vector4() {}
#pragma warning(pop)

	INLINE Vector4(float x, float y, float z, float w) { m_vec = XMVectorSet(x, y, z, w); }
	INLINE Vector4(const XMFLOAT4& v) { m_vec = XMLoadFloat4(&v); }
	INLINE Vector4(Vector3 xyz, Scalar w) { m_vec = XMVectorSetW(xyz, (float)w); }
	INLINE Vector4(const Vector4& v) { m_vec = v; }
	INLINE Vector4(const Scalar& s) { m_vec = s; }
	INLINE explicit Vector4(Vector3 xyz) { m_vec = SetWToOne(xyz); }
	INLINE explicit Vector4(FXMVECTOR vec) { m_vec = vec; }
	INLINE explicit Vector4(EZeroTag) { m_vec = SplatZero(); }
	INLINE explicit Vector4(EIdentityTag) { m_vec = SplatOne(); }
	INLINE explicit Vector4(EXUnitVector) { m_vec = CreateXUnitVector(); }
	INLINE explicit Vector4(EYUnitVector) { m_vec = CreateYUnitVector(); }
	INLINE explicit Vector4(EZUnitVector) { m_vec = CreateZUnitVector(); }
	INLINE explicit Vector4(EWUnitVector) { m_vec = CreateWUnitVector(); }

	INLINE Vector4& operator=(const Vector4& v) { m_vec = v; return *this; }

	INLINE operator XMVECTOR() const { return m_vec; }
	INLINE operator XMFLOAT4() const
	{
		XMFLOAT4 v;
		XMStoreFloat4(&v, m_vec);
		return v;
	}

	INLINE Scalar GetX() const { return Scalar(XMVectorSplatX(m_vec)); }
	INLINE Scalar GetY() const { return Scalar(XMVectorSplatY(m_vec)); }
	INLINE Scalar GetZ() const { return Scalar(XMVectorSplatZ(m_vec)); }
	INLINE Scalar GetW() const { return Scalar(XMVectorSplatW(m_vec)); }
	INLINE void SetX(Scalar x) { m_vec = XMVectorPermute<4, 1, 2, 3>(m_vec, x); }
	INLINE void SetY(Scalar y) { m_vec = XMVectorPermute<0, 5, 2, 3>(m_vec, y); }
	INLINE void SetZ(Scalar z) { m_vec = XMVectorPermute<0, 1, 6, 3>(m_vec, z); }
	INLINE void SetW(Scalar w) { m_vec = XMVectorPermute<0, 1, 2, 7>(m_vec, w); }
	INLINE void SetXYZ(Vector3 xyz) { m_vec = XMVectorPermute<0, 1, 2, 7>(xyz, m_vec); }

	INLINE Vector4 operator-() const { return Vector4(XMVectorNegate(m_vec)); }
	INLINE Vector4 operator+(Vector4 v2) const { return Vector4(XMVectorAdd(m_vec, v2)); }
	INLINE Vector4 operator-(Vector4 v2) const { return Vector4(XMVectorSubtract(m_vec, v2)); }
	INLINE Scalar operator*(Vector4 v2) const { return Scalar(XMVector4Dot(m_vec, v2)); }
	INLINE Vector4 operator/(Vector4 v2) const { return Vector4(XMVectorDivide(m_vec, v2)); }
	INLINE Vector4 operator*(Scalar v2) const { return *this * Vector4(v2); }
	INLINE Vector4 operator/(Scalar v2) const { return *this / Vector4(v2); }
	INLINE Vector4 operator*(float v2) const { return *this * Scalar(v2); }
	INLINE Vector4 operator/(float v2) const { return *this / Scalar(v2); }
	INLINE Scalar operator~() const { return Scalar(XMVector3Length(m_vec)); }

	INLINE void operator*=(float v2) { *this = *this * Scalar(v2); }
	INLINE void operator/=(float v2) { *this = *this / Scalar(v2); }
	INLINE Scalar operator[](int pos) const
	{
		if (pos == 0)
			return GetX();
		else if (pos == 1)
			return GetY();
		else if (pos == 2)
			return GetZ();
		else if (pos == 3)
			return GetW();
		else
			throw std::runtime_error("Indexul a depasit dimensiunea vectorului: index - " + std::to_string(pos));
	}

	INLINE friend Vector4 operator*(Scalar v1, Vector4 v2) { return Vector4(v1) * v2; }
	INLINE friend Vector4 operator/(Scalar v1, Vector4 v2) { return Vector4(v1) / v2; }
	INLINE friend Vector4 operator*(float v1, Vector4 v2) { return Scalar(v1) * v2; }
	INLINE friend Vector4 operator/(float v1, Vector4 v2) { return Scalar(v1) / v2; }

protected:
	XMVECTOR m_vec;
};

// Defined after Vector4 methods are declared
INLINE Vector3::Vector3(Vector4 vec) : m_vec((XMVECTOR)vec)
{
}
INLINE void Vector3::operator=(Vector4 vec)
{
	m_vec = (XMVECTOR)vec;
}

// For W != 1, divide XYZ by W.  If W == 0, do nothing
INLINE Vector3 MakeHomogeneous(Vector4 v)
{
	Scalar W = v.GetW();
	return Vector3(XMVectorSelect(XMVectorDivide(v, W), v, XMVectorEqual(W, SplatZero())));
}

class BoolVector
{
public:
	INLINE BoolVector(FXMVECTOR vec) { m_vec = vec; }
	INLINE operator XMVECTOR() const { return m_vec; }

protected:
	XMVECTOR m_vec;
};

typedef Vector3 Point3;
typedef Vector4 Point4;

}  // namespace Math
