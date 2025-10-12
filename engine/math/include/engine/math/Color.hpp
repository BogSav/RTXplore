//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard
//

#pragma once

#include "Common.hpp"

namespace engine::math
{

class Color
{
public:
	INLINE Color() : m_value(g_XMOne) {}
	Color(FXMVECTOR vec);
	Color(const XMVECTORF32& vec);
	INLINE Color(float r, float g, float b, float a = 1.0f) { m_value.v = XMVectorSet(r, g, b, a); }
	Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 255, uint16_t bitDepth = 8);
	explicit Color(uint32_t rgbaLittleEndian);

	INLINE float R() const { return XMVectorGetX(m_value.v); }
	INLINE float G() const { return XMVectorGetY(m_value.v); }
	INLINE float B() const { return XMVectorGetZ(m_value.v); }
	INLINE float A() const { return XMVectorGetW(m_value.v); }

	INLINE bool operator==(const Color& rhs) const { return XMVector4Equal(m_value.v, rhs.m_value.v); }
	INLINE bool operator!=(const Color& rhs) const { return !XMVector4Equal(m_value.v, rhs.m_value.v); }

	INLINE void SetR(float r) { m_value.f[0] = r; }
	INLINE void SetG(float g) { m_value.f[1] = g; }
	INLINE void SetB(float b) { m_value.f[2] = b; }
	INLINE void SetA(float a) { m_value.f[3] = a; }

	INLINE float* GetPtr() { return reinterpret_cast<float*>(this); }
	INLINE const float* GetPtr() const { return reinterpret_cast<const float*>(this); }
	INLINE float& operator[](int idx) { return GetPtr()[idx]; }

	INLINE void SetRGB(float r, float g, float b) { m_value.v = XMVectorSelect(m_value.v, XMVectorSet(r, g, b, b), g_XMMask3.v); }

	Color ToSRGB() const;
	Color FromSRGB() const;
	Color ToREC709() const;
	Color FromREC709() const;

	uint32_t R10G10B10A2() const;
	uint32_t R8G8B8A8() const;

	INLINE operator XMVECTOR() const { return m_value.v; }

private:
	XMVECTORF32 m_value;
};

INLINE Color Max(Color a, Color b) { return Color(XMVectorMax(a, b)); }
INLINE Color Min(Color a, Color b) { return Color(XMVectorMin(a, b)); }
INLINE Color Clamp(Color x, Color a, Color b) { return Color(XMVectorClamp(x, a, b)); }

}  // namespace engine::math
