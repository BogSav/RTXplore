#include "GeometryHelper.hpp"

void GeometryHelper::ComputeVertexNormalsAndTangents(Mesh::Ptr mesh)
{
	for (auto& vertex : mesh->m_vertices)
	{
		vertex.normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex.tangent = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	for (size_t i = 0; i < mesh->m_indices.size(); i += 3)
	{
		const auto& p0 = mesh->m_vertices[mesh->m_indices[i]].position;
		const auto& p1 = mesh->m_vertices[mesh->m_indices[i + 1]].position;
		const auto& p2 = mesh->m_vertices[mesh->m_indices[i + 2]].position;

		const auto& uv0 = mesh->m_vertices[mesh->m_indices[i]].texC;
		const auto& uv1 = mesh->m_vertices[mesh->m_indices[i + 1]].texC;
		const auto& uv2 = mesh->m_vertices[mesh->m_indices[i + 2]].texC;

		DirectX::XMVECTOR v0 = DirectX::XMLoadFloat3(&p0);
		DirectX::XMVECTOR v1 = DirectX::XMLoadFloat3(&p1);
		DirectX::XMVECTOR v2 = DirectX::XMLoadFloat3(&p2);

		DirectX::XMVECTOR u = DirectX::XMVectorSubtract(v1, v0);
		DirectX::XMVECTOR v = DirectX::XMVectorSubtract(v2, v0);

		DirectX::XMVECTOR normal = DirectX::XMVector3Cross(u, v);

		float deltaU1 = uv1.x - uv0.x;
		float deltaV1 = uv1.y - uv0.y;
		float deltaU2 = uv2.x - uv0.x;
		float deltaV2 = uv2.y - uv0.y;

		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		DirectX::XMFLOAT3 tangent;
		tangent.x = f * (deltaV2 * DirectX::XMVectorGetX(u) - deltaV1 * DirectX::XMVectorGetX(v));
		tangent.y = f * (deltaV2 * DirectX::XMVectorGetY(u) - deltaV1 * DirectX::XMVectorGetY(v));
		tangent.z = f * (deltaV2 * DirectX::XMVectorGetZ(u) - deltaV1 * DirectX::XMVectorGetZ(v));

		for (int j = 0; j < 3; j++)
		{
			DirectX::XMVECTOR existingNormal = DirectX::XMLoadFloat3(&mesh->m_vertices[mesh->m_indices[i + j]].normal);
			DirectX::XMVECTOR updatedNormal = DirectX::XMVectorAdd(existingNormal, normal);
			DirectX::XMStoreFloat3(&mesh->m_vertices[mesh->m_indices[i + j]].normal, updatedNormal);

			DirectX::XMVECTOR existingTangent =
				DirectX::XMLoadFloat3(&mesh->m_vertices[mesh->m_indices[i + j]].tangent);
			DirectX::XMVECTOR updatedTangent = DirectX::XMVectorAdd(existingTangent, XMLoadFloat3(&tangent));
			DirectX::XMStoreFloat3(&mesh->m_vertices[mesh->m_indices[i + j]].tangent, updatedTangent);
		}
	}

	// Normalizăm normalele și tangentelor
	for (auto& vertex : mesh->m_vertices)
	{
		DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&vertex.normal);
		normal = DirectX::XMVector3Normalize(normal);
		DirectX::XMStoreFloat3(&vertex.normal, normal);

		DirectX::XMVECTOR tangent = DirectX::XMLoadFloat3(&vertex.tangent);
		tangent = DirectX::XMVector3Normalize(tangent);
		DirectX::XMStoreFloat3(&vertex.tangent, tangent);
	}
}

void GeometryHelper::ProjectVerticesOntoSphere(Mesh::Ptr mesh, float radius)
{
	using namespace DirectX;

	for (std::uint32_t i = 0; i < mesh->GetVertexCount(); i++)
	{
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&mesh->m_vertices[i].position));
		XMVECTOR p = radius * n;

		XMStoreFloat3(&mesh->m_vertices[i].position, p);
		XMStoreFloat3(&mesh->m_vertices[i].normal, n);
	}
}

void GeometryHelper::MoveVerticesToPosition(Mesh::Ptr mesh, Math::Vector3 position)
{
	using namespace DirectX;

	for (std::uint32_t i = 0; i < mesh->GetVertexCount(); i++)
	{
		XMVECTOR p = XMLoadFloat3(&mesh->m_vertices[i].position);
		XMStoreFloat3(&mesh->m_vertices[i].position, p + position);
	}
}

void GeometryHelper::ApplyHeightFunctionForGrid(Mesh::Ptr mesh, std::function<float(float, float)> heightFunction)
{
	using namespace DirectX;

	for (auto& vertex : mesh->m_vertices)
	{
		vertex.position.y = heightFunction(vertex.position.x, vertex.position.z);
	}
}

Mesh::Vertex GeometryHelper::GetInterpolatedVertex(const Mesh::Vertex& v0, const Mesh::Vertex& v1, float t)
{
	using namespace DirectX;

	XMVECTOR p0 = XMLoadFloat3(&v0.position);
	XMVECTOR p1 = XMLoadFloat3(&v1.position);

	XMVECTOR c0 = XMLoadFloat4(&v0.color);
	XMVECTOR c1 = XMLoadFloat4(&v1.color);

	XMVECTOR n0 = XMLoadFloat3(&v0.normal);
	XMVECTOR n1 = XMLoadFloat3(&v1.normal);

	XMVECTOR tan0 = XMLoadFloat3(&v0.tangent);
	XMVECTOR tan1 = XMLoadFloat3(&v1.tangent);

	XMVECTOR tex0 = XMLoadFloat2(&v0.texC);
	XMVECTOR tex1 = XMLoadFloat2(&v1.texC);

	XMVECTOR pos = t * (p0 + p1);
	XMVECTOR normal = XMVector3Normalize(t * (n0 + n1));
	XMVECTOR tangent = XMVector3Normalize(t * (tan0 + tan1));
	XMVECTOR tex = t * (tex0 + tex1);
	XMVECTOR color = t * (c0 + c1);

	Mesh::Vertex v;
	XMStoreFloat3(&v.position, pos);
	XMStoreFloat3(&v.normal, normal);
	XMStoreFloat3(&v.tangent, tangent);
	XMStoreFloat2(&v.texC, tex);
	XMStoreFloat4(&v.color, color);

	return v;
}

void GeometryHelper::Subdivide(Mesh::Ptr mesh, int nrOfSubdivisions)
{
	if (nrOfSubdivisions <= 0)
		return;

	std::vector<Mesh::Vertex> newVertices = mesh->m_vertices;
	std::vector<Mesh::Index> newIndices;

	std::unordered_map<std::string, Mesh::Index> midpointCache;

	std::uint32_t numTris = (std::uint32_t)mesh->GetIndexCount() / 3;
	newVertices.reserve(newVertices.size() + numTris * 3);

	for (std::uint32_t i = 0; i < numTris; ++i)
	{
		std::uint32_t offset = i * 3;

		Mesh::Index index0 = mesh->m_indices[offset + 0];
		Mesh::Index index1 = mesh->m_indices[offset + 1];
		Mesh::Index index2 = mesh->m_indices[offset + 2];

		Mesh::Index mIndex0 = FindOrCreateMidpoint(index0, index1, midpointCache, mesh, newVertices);
		Mesh::Index mIndex1 = FindOrCreateMidpoint(index1, index2, midpointCache, mesh, newVertices);
		Mesh::Index mIndex2 = FindOrCreateMidpoint(index0, index2, midpointCache, mesh, newVertices);

		newIndices.push_back(index0);
		newIndices.push_back(mIndex0);
		newIndices.push_back(mIndex2);

		newIndices.push_back(index1);
		newIndices.push_back(mIndex1);
		newIndices.push_back(mIndex0);

		newIndices.push_back(index2);
		newIndices.push_back(mIndex2);
		newIndices.push_back(mIndex1);

		newIndices.push_back(mIndex0);
		newIndices.push_back(mIndex1);
		newIndices.push_back(mIndex2);
	}

	mesh->m_vertices.swap(newVertices);
	mesh->m_indices.swap(newIndices);

	Subdivide(mesh, nrOfSubdivisions - 1);
}

void GeometryHelper::TransformTextureCoordinates(Mesh::Ptr mesh, float terrainWidth, float terrainLength)
{
	using namespace DirectX;

	const float halfLength = terrainLength / 2.f;
	const float helfWidth = terrainWidth / 2.f;

	for (auto& vertex : mesh->m_vertices)
	{
		vertex.texC =
			XMFLOAT2((vertex.position.x + halfLength) / terrainLength, (vertex.position.z + helfWidth) / terrainWidth);
	}
}

void GeometryHelper::ChnageColor(Mesh::Ptr mesh, Math::Vector4 color)
{
	using namespace DirectX;

	for (auto& vertex : mesh->m_vertices)
	{
		vertex.color = color;
	}
}

std::string GeometryHelper::GetEdgeKey(Mesh::Index a, Mesh::Index b)
{
	if (a > b)
	{
		std::swap(a, b);
	}
	return std::to_string(a) + "_" + std::to_string(b);
}

Mesh::Index GeometryHelper::FindOrCreateMidpoint(
	Mesh::Index indexA,
	Mesh::Index indexB,
	std::unordered_map<std::string, Mesh::Index>& midpointCache,
	Mesh::Ptr mesh,
	std::vector<Mesh::Vertex>& newVertices)
{
	std::string edgeKey = GetEdgeKey(indexA, indexB);

	auto it = midpointCache.find(edgeKey);
	if (it != midpointCache.end())
	{
		return it->second;
	}

	const Mesh::Vertex& vertexA = mesh->m_vertices[indexA];
	const Mesh::Vertex& vertexB = mesh->m_vertices[indexB];

	Mesh::Vertex midpoint = GetInterpolatedVertex(vertexA, vertexB);
	Mesh::Index midIndex = static_cast<Mesh::Index>(newVertices.size());

	newVertices.push_back(midpoint);

	midpointCache[edgeKey] = midIndex;

	return midIndex;
}