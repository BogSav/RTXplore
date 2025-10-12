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

#include "Color.hpp"

namespace engine::math
{

Color::Color(FXMVECTOR vec)
{
	m_value.v = vec;
}

Color::Color(const XMVECTORF32& vec)
{
	m_value = vec;
}

Color::Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a, uint16_t bitDepth)
{
	m_value.v = XMVectorScale(XMVectorSet((float)r, (float)g, (float)b, (float)a), 1.0f / ((1 << bitDepth) - 1));
}

Color::Color(uint32_t u32)
{
	float r = (float)((u32 >> 0) & 0xFF);
	float g = (float)((u32 >> 8) & 0xFF);
	float b = (float)((u32 >> 16) & 0xFF);
	float a = (float)((u32 >> 24) & 0xFF);
	m_value.v = XMVectorScale(XMVectorSet(r, g, b, a), 1.0f / 255.0f);
}

Color Color::ToSRGB() const
{
	XMVECTOR T = XMVectorSaturate(m_value.v);
	XMVECTOR result = XMVectorSubtract(
		XMVectorScale(XMVectorPow(T, XMVectorReplicate(1.0f / 2.4f)), 1.055f), XMVectorReplicate(0.055f));
	result = XMVectorSelect(result, XMVectorScale(T, 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return Color(XMVectorSelect(T, result, g_XMSelect1110.v));
}

Color Color::FromSRGB() const
{
	XMVECTOR T = XMVectorSaturate(m_value.v);
	XMVECTOR result =
		XMVectorPow(XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.055f)), 1.0f / 1.055f), XMVectorReplicate(2.4f));
	result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 12.92f), XMVectorLess(T, XMVectorReplicate(0.0031308f)));
	return Color(XMVectorSelect(T, result, g_XMSelect1110.v));
}

Color Color::ToREC709() const
{
	XMVECTOR T = XMVectorSaturate(m_value.v);
	XMVECTOR result =
		XMVectorSubtract(XMVectorScale(XMVectorPow(T, XMVectorReplicate(0.45f)), 1.099f), XMVectorReplicate(0.099f));
	result = XMVectorSelect(result, XMVectorScale(T, 4.5f), XMVectorLess(T, XMVectorReplicate(0.0018f)));
	return Color(XMVectorSelect(T, result, g_XMSelect1110.v));
}

Color Color::FromREC709() const
{
	XMVECTOR T = XMVectorSaturate(m_value.v);
	XMVECTOR result = XMVectorPow(
		XMVectorScale(XMVectorAdd(T, XMVectorReplicate(0.099f)), 1.0f / 1.099f), XMVectorReplicate(1.0f / 0.45f));
	result = XMVectorSelect(result, XMVectorScale(T, 1.0f / 4.5f), XMVectorLess(T, XMVectorReplicate(0.0081f)));
	return Color(XMVectorSelect(T, result, g_XMSelect1110.v));
}

uint32_t Color::R10G10B10A2() const
{
	XMVECTOR result =
		XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value.v), XMVectorSet(1023.0f, 1023.0f, 1023.0f, 3.0f)));
	result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
	uint32_t r = XMVectorGetIntX(result);
	uint32_t g = XMVectorGetIntY(result);
	uint32_t b = XMVectorGetIntZ(result);
	uint32_t a = XMVectorGetIntW(result) >> 8;
	return a << 30 | b << 20 | g << 10 | r;
}

uint32_t Color::R8G8B8A8() const
{
	XMVECTOR result = XMVectorRound(XMVectorMultiply(XMVectorSaturate(m_value.v), XMVectorReplicate(255.0f)));
	result = _mm_castsi128_ps(_mm_cvttps_epi32(result));
	uint32_t r = XMVectorGetIntX(result);
	uint32_t g = XMVectorGetIntY(result);
	uint32_t b = XMVectorGetIntZ(result);
	uint32_t a = XMVectorGetIntW(result);
	return a << 24 | b << 16 | g << 8 | r;
}

}  // namespace engine::math
