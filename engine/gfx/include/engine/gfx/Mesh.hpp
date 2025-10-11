#pragma once

#include "engine/math/AxisAllignedBBox.hpp"
#include "engine/core/customException.hpp"

#include <vector>

class Mesh
{
public:
	using Ptr = std::shared_ptr<Mesh>;
	using Index = std::uint32_t;

	struct Vertex
	{
		Vertex() noexcept = default;
		Vertex(DirectX::XMFLOAT3 pos) noexcept : position(pos){};
		Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color) noexcept : position(pos), color(color) {}
		Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT4 color, DirectX::XMFLOAT2 texC) noexcept
			: position(pos), color(color), texC(texC){};

		DirectX::XMFLOAT3 position = {0.f, 0.f, 0.f};
		DirectX::XMFLOAT4 color = {1.f, 1.f, 1.f, 1.f};
		DirectX::XMFLOAT3 normal = {0.f, 0.f, 0.f};
		DirectX::XMFLOAT3 tangent = {0.f, 0.f, 0.f};
		DirectX::XMFLOAT2 texC = {0.f, 0.f};
	};

public:
	Mesh() noexcept = default;
	Mesh(std::vector<Vertex> vertices, std::vector<Index> indices) noexcept : m_vertices(vertices), m_indices(indices)
	{
	}

	void Concatenate(Mesh::Ptr meshPtr)
	{
		m_vertices.insert(m_vertices.end(), meshPtr->GetVertexVector().cbegin(), meshPtr->GetVertexVector().cend());
		m_indices.insert(m_indices.end(), meshPtr->GetIndexVector().cbegin(), meshPtr->GetIndexVector().cend());
	}

	void SetVertices(std::initializer_list<Vertex> vertices) noexcept
	{
		m_vertices.reserve(vertices.size());
		m_vertices = std::vector<Vertex>(vertices);
	}
	void SetIndices(std::initializer_list<Index> indices) noexcept
	{
		m_indices.reserve(indices.size());
		m_indices = std::vector<Index>(indices);
	}

	const Vertex* GetVerticesData() const
	{
		if (m_vertices.empty())
			throw misc::customException("Nu exista vertexi in mesh");

		return &m_vertices[0];
	}
	const Index* GetIndicesData() const
	{
		if (m_indices.empty())
			throw misc::customException("Nu exista indecsi in mesh");

		return &m_indices[0];
	}

	Math::AABB GetAABB() const
	{
		Math::AABB bbox;

		for (auto& vertex : m_vertices)
		{
			bbox.EnlargeForPoint(vertex.position);
		}

		return bbox;
	}

	static inline constexpr size_t GetSizeOfVertex() noexcept { return sizeof(Vertex); }
	static inline constexpr size_t GetSizeOfIndex() noexcept { return sizeof(Index); }

	inline size_t GetVerticesDataSize() const noexcept { return sizeof(Vertex) * m_vertices.size(); }
	inline size_t GetIndicesDataSize() const noexcept { return sizeof(Index) * m_indices.size(); }
	inline size_t GetIndexCount() const noexcept { return m_indices.size(); }
	inline size_t GetVertexCount() const noexcept { return m_vertices.size(); }

	inline const std::vector<Vertex>& GetVertexVector() const { return m_vertices; }
	inline const std::vector<Index>& GetIndexVector() const { return m_indices; }

private:
	friend struct GeometryHelper;

	std::vector<Vertex> m_vertices;
	std::vector<Index> m_indices;
};

struct SubMesh
{
	size_t indexCount;
	size_t startIndexLocation;
	size_t baseVertexLocation;
};
