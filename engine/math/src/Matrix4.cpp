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

#include "Matrix4.hpp"

namespace Math
{

Matrix4 Matrix4::Transpose(const Matrix4& m)
{
	return Matrix4(XMMatrixTranspose(m));
}

Matrix4 Matrix4::Inverse(const Matrix4& m)
{
	XMVECTOR determinant = XMMatrixDeterminant(m);
	return Matrix4(XMMatrixInverse(&determinant, m));
}

}  // namespace Math
