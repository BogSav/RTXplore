#pragma once

#include "Common.hpp"

using namespace DirectX;

namespace engine::math {

class alignas(16) Scalar {
public:
    // tipuri "tag" opționale
    struct ZeroTag{};  static constexpr ZeroTag Zero{};
    struct OneTag{};   static constexpr OneTag  One{};

    // Ctors
    INLINE Scalar() noexcept : m_vec(XMVectorZero()) {}
    INLINE Scalar(const Scalar&) noexcept = default;
    INLINE Scalar(Scalar&&) noexcept = default;

    INLINE explicit Scalar(float f) noexcept : m_vec(XMVectorReplicate(f)) {}
    INLINE explicit Scalar(FXMVECTOR v) noexcept : m_vec(v) {}
    INLINE Scalar(ZeroTag) noexcept : m_vec(XMVectorZero()) {}
    INLINE Scalar(OneTag)  noexcept : m_vec(XMVectorSplatOne()) {}

    // Assign
    INLINE Scalar& operator=(const Scalar&) noexcept = default;
    INLINE Scalar& operator=(Scalar&&) noexcept = default;

    // Conversii
    INLINE /*implicit*/ operator XMVECTOR() const noexcept { return m_vec; }
    INLINE explicit operator float() const noexcept { return XMVectorGetX(m_vec); }

    // Acces raw
    INLINE XMVECTOR vec() const noexcept { return m_vec; }

    // Operatori compuși
    INLINE Scalar& operator+=(const Scalar& s) noexcept { m_vec = XMVectorAdd(m_vec, s.m_vec); return *this; }
    INLINE Scalar& operator-=(const Scalar& s) noexcept { m_vec = XMVectorSubtract(m_vec, s.m_vec); return *this; }
    INLINE Scalar& operator*=(const Scalar& s) noexcept { m_vec = XMVectorMultiply(m_vec, s.m_vec); return *this; }
    INLINE Scalar& operator/=(const Scalar& s) noexcept { m_vec = XMVectorDivide(m_vec, s.m_vec); return *this; }

    INLINE Scalar& operator+=(float f) noexcept { m_vec = XMVectorAdd(m_vec, XMVectorReplicate(f)); return *this; }
    INLINE Scalar& operator-=(float f) noexcept { m_vec = XMVectorSubtract(m_vec, XMVectorReplicate(f)); return *this; }
    INLINE Scalar& operator*=(float f) noexcept { m_vec = XMVectorScale(m_vec, f); return *this; }
    INLINE Scalar& operator/=(float f) noexcept { m_vec = XMVectorDivide(m_vec, XMVectorReplicate(f)); return *this; }

    // Helpers
    INLINE static Scalar Sqrt(Scalar s) noexcept { return Scalar(XMVectorSqrt(s.m_vec)); }
    INLINE static Scalar Rsqrt(Scalar s) noexcept { return Scalar(XMVectorReciprocalSqrt(s.m_vec)); }
    INLINE static Scalar RsqrtEst(Scalar s) noexcept { return Scalar(XMVectorReciprocalSqrtEst(s.m_vec)); }
    INLINE static Scalar Rcp(Scalar s) noexcept { return Scalar(XMVectorReciprocal(s.m_vec)); }
    INLINE static Scalar Abs(Scalar s) noexcept { return Scalar(XMVectorAbs(s.m_vec)); }
    INLINE static Scalar Min(Scalar a, Scalar b) noexcept { return Scalar(XMVectorMin(a.m_vec, b.m_vec)); }
    INLINE static Scalar Max(Scalar a, Scalar b) noexcept { return Scalar(XMVectorMax(a.m_vec, b.m_vec)); }
    INLINE static Scalar Saturate(Scalar s) noexcept { return Scalar(XMVectorSaturate(s.m_vec)); }

private:
    XMVECTOR m_vec;
};

// Operatori liberi (evită temporare inutile)
INLINE Scalar operator-(Scalar s) noexcept { return Scalar(XMVectorNegate(s)); }

INLINE Scalar operator+(Scalar a, Scalar b) noexcept { return Scalar(XMVectorAdd(a, b)); }
INLINE Scalar operator-(Scalar a, Scalar b) noexcept { return Scalar(XMVectorSubtract(a, b)); }
INLINE Scalar operator*(Scalar a, Scalar b) noexcept { return Scalar(XMVectorMultiply(a, b)); }
INLINE Scalar operator/(Scalar a, Scalar b) noexcept { return Scalar(XMVectorDivide(a, b)); }

INLINE Scalar operator+(Scalar a, float b) noexcept { return Scalar(XMVectorAdd(a, XMVectorReplicate(b))); }
INLINE Scalar operator-(Scalar a, float b) noexcept { return Scalar(XMVectorSubtract(a, XMVectorReplicate(b))); }
INLINE Scalar operator*(Scalar a, float b) noexcept { return Scalar(XMVectorScale(a, b)); }
INLINE Scalar operator/(Scalar a, float b) noexcept { return Scalar(XMVectorDivide(a, XMVectorReplicate(b))); }

INLINE Scalar operator+(float a, Scalar b) noexcept { return Scalar(XMVectorAdd(XMVectorReplicate(a), b)); }
INLINE Scalar operator-(float a, Scalar b) noexcept { return Scalar(XMVectorSubtract(XMVectorReplicate(a), b)); }
INLINE Scalar operator*(float a, Scalar b) noexcept { return Scalar(XMVectorScale(b, a)); }
INLINE Scalar operator/(float a, Scalar b) noexcept { return Scalar(XMVectorDivide(XMVectorReplicate(a), b)); }

} // namespace engine::math
