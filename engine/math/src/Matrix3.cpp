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

#include "Matrix3.hpp"

namespace Math
{

Matrix3 Matrix3::Inverse(const Matrix3& m)
{
	XMVECTOR determinant = XMMatrixDeterminant(m);
	return Matrix3(XMMatrixInverse(&determinant, m));
}

}  // namespace Math
