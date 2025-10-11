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

#include "AxisAllignedBBox.hpp"

namespace Math
{

std::string AABB::ToString() const
{
	if (!IsInitialized())
	{
		return "Not initialized";
	}
	else
	{
		return "BLN: " + GetMin().ToString() + " URF: " + GetMax().ToString();
	}
}

AABB& AABB::Transform(const Matrix4& m)
{
	if (!IsInitialized())
	{
		return *this;
	}

	Vector3 c[8];
	c[0] = m_minCorner;
	c[1] = m_maxCorner;
	c[2] = Vector3(m_minCorner.GetX(), m_minCorner.GetY(), m_maxCorner.GetZ());
	c[3] = Vector3(m_minCorner.GetX(), m_maxCorner.GetY(), m_maxCorner.GetZ());
	c[4] = Vector3(m_maxCorner.GetX(), m_minCorner.GetY(), m_maxCorner.GetZ());
	c[5] = Vector3(m_minCorner.GetX(), m_maxCorner.GetY(), m_minCorner.GetZ());
	c[6] = Vector3(m_maxCorner.GetX(), m_minCorner.GetY(), m_minCorner.GetZ());
	c[7] = Vector3(m_maxCorner.GetX(), m_maxCorner.GetY(), m_minCorner.GetZ());
	
	for (int i = 0; i < 8; ++i)
	{
		c[i] = c[i] * m;
	}
	
	SetCorners(c[0], c[1]);
	for (int j = 2; j < 8; ++j)
	{
		EnlargeForPoint(c[j]);
	}
	
	return *this;
}

}  // namespace Math
