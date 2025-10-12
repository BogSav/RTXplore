#include "GeometryHelper.hpp"

#include <DirectXMath.h>

#include <algorithm>
#include <cmath>
#include <vector>

using namespace DirectX;

namespace engine::gfx
{

void GeometryHelper::ComputeVertexNormalsAndTangents(Mesh::Ptr mesh)
{
	if (!mesh)
		return;

	auto &vertices = mesh->m_vertices;
	auto &indices = mesh->m_indices;

	if (vertices.empty() || indices.empty())
		return;

	for (auto &vertex : vertices)
	{
		vertex.normal = {0.f, 0.f, 0.f};
		vertex.tangent = {0.f, 0.f, 0.f};
	}

	const size_t triangleCount = indices.size() / 3;
	for (size_t t = 0; t < triangleCount; ++t)
	{
		const Mesh::Index i0 = indices[t * 3 + 0];
		const Mesh::Index i1 = indices[t * 3 + 1];
		const Mesh::Index i2 = indices[t * 3 + 2];

		Mesh::Vertex &v0 = vertices[i0];
		Mesh::Vertex &v1 = vertices[i1];
		Mesh::Vertex &v2 = vertices[i2];

		const XMVECTOR p0 = XMLoadFloat3(&v0.position);
		const XMVECTOR p1 = XMLoadFloat3(&v1.position);
		const XMVECTOR p2 = XMLoadFloat3(&v2.position);

		const XMVECTOR edge1 = XMVectorSubtract(p1, p0);
		const XMVECTOR edge2 = XMVectorSubtract(p2, p0);

		XMVECTOR faceNormal = XMVector3Cross(edge1, edge2);
		faceNormal = XMVector3Normalize(faceNormal);

		const float du1 = v1.texC.x - v0.texC.x;
		const float dv1 = v1.texC.y - v0.texC.y;
		const float du2 = v2.texC.x - v0.texC.x;
		const float dv2 = v2.texC.y - v0.texC.y;

		XMVECTOR faceTangent;
		const float denom = du1 * dv2 - du2 * dv1;
		if (std::fabs(denom) > 1e-8f)
		{
			const float invDenom = 1.f / denom;
			const XMVECTOR scaledEdge1 = XMVectorScale(edge1, dv2);
			const XMVECTOR scaledEdge2 = XMVectorScale(edge2, dv1);
			faceTangent = XMVectorScale(XMVectorSubtract(scaledEdge1, scaledEdge2), invDenom);
		}
		else
		{
			faceTangent = XMVector3Normalize(edge1);
		}

		XMFLOAT3 normalFloat;
		XMStoreFloat3(&normalFloat, faceNormal);

		XMFLOAT3 tangentFloat;
		XMStoreFloat3(&tangentFloat, faceTangent);

		auto accumulate = [](DirectX::XMFLOAT3 &dst, const DirectX::XMFLOAT3 &src) {
			dst.x += src.x;
			dst.y += src.y;
			dst.z += src.z;
		};

		accumulate(v0.normal, normalFloat);
		accumulate(v1.normal, normalFloat);
		accumulate(v2.normal, normalFloat);

		accumulate(v0.tangent, tangentFloat);
		accumulate(v1.tangent, tangentFloat);
		accumulate(v2.tangent, tangentFloat);
	}

	for (auto &vertex : vertices)
	{
		XMVECTOR normal = XMLoadFloat3(&vertex.normal);
		const float normalLenSq = XMVectorGetX(XMVector3LengthSq(normal));
		if (normalLenSq > 1e-12f)
		{
			normal = XMVector3Normalize(normal);
			XMStoreFloat3(&vertex.normal, normal);
		}

		XMVECTOR tangent = XMLoadFloat3(&vertex.tangent);
		const float tangentLenSq = XMVectorGetX(XMVector3LengthSq(tangent));
		if (tangentLenSq > 1e-12f)
		{
			tangent = XMVector3Normalize(tangent);
			XMStoreFloat3(&vertex.tangent, tangent);
		}
	}
}

void GeometryHelper::Subdivide(Mesh::Ptr mesh, int nrOfSubdivisions)
{
	if (!mesh || nrOfSubdivisions <= 0)
		return;

	for (int iteration = 0; iteration < nrOfSubdivisions; ++iteration)
	{
		std::unordered_map<std::string, Mesh::Index> midpointCache;
		std::vector<Mesh::Vertex> newVertices = mesh->m_vertices;
		std::vector<Mesh::Index> newIndices;
		newIndices.reserve(mesh->m_indices.size() * 4);

		for (size_t idx = 0; idx < mesh->m_indices.size(); idx += 3)
		{
			Mesh::Index i0 = mesh->m_indices[idx + 0];
			Mesh::Index i1 = mesh->m_indices[idx + 1];
			Mesh::Index i2 = mesh->m_indices[idx + 2];

			Mesh::Index m0 = FindOrCreateMidpoint(i0, i1, midpointCache, mesh, newVertices);
			Mesh::Index m1 = FindOrCreateMidpoint(i1, i2, midpointCache, mesh, newVertices);
			Mesh::Index m2 = FindOrCreateMidpoint(i2, i0, midpointCache, mesh, newVertices);

			newIndices.push_back(i0);
			newIndices.push_back(m0);
			newIndices.push_back(m2);

			newIndices.push_back(i1);
			newIndices.push_back(m1);
			newIndices.push_back(m0);

			newIndices.push_back(i2);
			newIndices.push_back(m2);
			newIndices.push_back(m1);

			newIndices.push_back(m0);
			newIndices.push_back(m1);
			newIndices.push_back(m2);
		}

		mesh->m_vertices = std::move(newVertices);
		mesh->m_indices = std::move(newIndices);
	}
}

void GeometryHelper::ProjectVerticesOntoSphere(Mesh::Ptr mesh, float radius)
{
	if (!mesh || radius <= 0.f)
		return;

	for (auto &vertex : mesh->m_vertices)
	{
		XMVECTOR position = XMLoadFloat3(&vertex.position);
		position = XMVector3Normalize(position);
		position = XMVectorScale(position, radius);
		XMStoreFloat3(&vertex.position, position);
	}
}

void GeometryHelper::MoveVerticesToPosition(Mesh::Ptr mesh, engine::math::Vector3 position)
{
	if (!mesh)
		return;

	const float offsetX = static_cast<float>(position.GetX());
	const float offsetY = static_cast<float>(position.GetY());
	const float offsetZ = static_cast<float>(position.GetZ());

	for (auto &vertex : mesh->m_vertices)
	{
		vertex.position.x += offsetX;
		vertex.position.y += offsetY;
		vertex.position.z += offsetZ;
	}
}

void GeometryHelper::ApplyHeightFunctionForGrid(Mesh::Ptr mesh, std::function<float(float, float)> heightFunction)
{
	if (!mesh || !heightFunction)
		return;

	for (auto &vertex : mesh->m_vertices)
	{
		const float x = vertex.position.x;
		const float z = vertex.position.z;
		vertex.position.y = heightFunction(x, z);
	}
}

void GeometryHelper::TransformTextureCoordinates(Mesh::Ptr mesh, float width, float length)
{
	if (!mesh || width == 0.f || length == 0.f)
		return;

	const float halfWidth = width * 0.5f;
	const float halfLength = length * 0.5f;

	for (auto &vertex : mesh->m_vertices)
	{
		vertex.texC.x = (vertex.position.x + halfWidth) / width;
		vertex.texC.y = (vertex.position.z + halfLength) / length;
	}
}

void GeometryHelper::ChnageColor(Mesh::Ptr mesh, engine::math::Vector4 color)
{
	if (!mesh)
		return;

	XMFLOAT4 colorFloat(
		static_cast<float>(color.GetX()),
		static_cast<float>(color.GetY()),
		static_cast<float>(color.GetZ()),
		static_cast<float>(color.GetW()));

	for (auto &vertex : mesh->m_vertices)
	{
		vertex.color = colorFloat;
	}
}

Mesh::Vertex GeometryHelper::GetInterpolatedVertex(const Mesh::Vertex &v0, const Mesh::Vertex &v1, float t)
{
	Mesh::Vertex result;

	auto lerp = [t](float a, float b) { return a + (b - a) * t; };

	result.position.x = lerp(v0.position.x, v1.position.x);
	result.position.y = lerp(v0.position.y, v1.position.y);
	result.position.z = lerp(v0.position.z, v1.position.z);

	result.color.x = lerp(v0.color.x, v1.color.x);
	result.color.y = lerp(v0.color.y, v1.color.y);
	result.color.z = lerp(v0.color.z, v1.color.z);
	result.color.w = lerp(v0.color.w, v1.color.w);

	result.normal.x = lerp(v0.normal.x, v1.normal.x);
	result.normal.y = lerp(v0.normal.y, v1.normal.y);
	result.normal.z = lerp(v0.normal.z, v1.normal.z);

	result.tangent.x = lerp(v0.tangent.x, v1.tangent.x);
	result.tangent.y = lerp(v0.tangent.y, v1.tangent.y);
	result.tangent.z = lerp(v0.tangent.z, v1.tangent.z);

	result.texC.x = lerp(v0.texC.x, v1.texC.x);
	result.texC.y = lerp(v0.texC.y, v1.texC.y);

	return result;
}

std::string GeometryHelper::GetEdgeKey(Mesh::Index a, Mesh::Index b)
{
	if (a > b)
		std::swap(a, b);
	return std::to_string(a) + "_" + std::to_string(b);
}

Mesh::Index GeometryHelper::FindOrCreateMidpoint(
	Mesh::Index indexA,
	Mesh::Index indexB,
	std::unordered_map<std::string, Mesh::Index> &midpointCache,
	Mesh::Ptr mesh,
	std::vector<Mesh::Vertex> &newVertices)
{
	if (indexA > indexB)
		std::swap(indexA, indexB);

	const std::string key = GetEdgeKey(indexA, indexB);
	auto it = midpointCache.find(key);
	if (it != midpointCache.end())
		return it->second;

	const Mesh::Vertex &v0 = mesh->m_vertices[indexA];
	const Mesh::Vertex &v1 = mesh->m_vertices[indexB];

	Mesh::Vertex midpoint = GetInterpolatedVertex(v0, v1);

	const Mesh::Index newIndex = static_cast<Mesh::Index>(newVertices.size());
	newVertices.push_back(midpoint);
	midpointCache.emplace(key, newIndex);

	return newIndex;
}

}  // namespace engine::gfx
