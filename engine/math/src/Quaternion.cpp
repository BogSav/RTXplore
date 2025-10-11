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

#include "Quaternion.hpp"

namespace Math
{

Quaternion UnsafeNormalize(Quaternion q)
{
	return Quaternion(XMQuaternionNormalize(q));
}

Quaternion Slerp(Quaternion a, Quaternion b, float t)
{
	return UnsafeNormalize(Quaternion(XMQuaternionSlerp(a, b, t)));
}

Quaternion Lerp(Quaternion a, Quaternion b, float t)
{
	return UnsafeNormalize(Quaternion(XMVectorLerp(a, b, t)));
}

}  // namespace Math
