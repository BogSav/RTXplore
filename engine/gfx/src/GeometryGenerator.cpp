#include "GeometryGenerator.hpp"

Mesh::Ptr GeometryGenerator::GenerateCylinder(
	const float bottomRadius, const float topRadius, const float height, const int stackCount, const int sliceCount)
{
	using namespace Math;

	Mesh::Ptr mesh;

	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	const float stackHeight = height / stackCount;
	const float radiusStep = (topRadius - bottomRadius) / stackCount;
	const float dr = bottomRadius - topRadius;
	const int ringCount = stackCount + 1;

	// Generare vertecsi suprafata
	for (int i = 0; i < ringCount; i++)
	{
		const Scalar y = Scalar(i) * stackHeight - height / 2.0f;
		const Scalar r = Scalar(bottomRadius) + i * radiusStep;

		const float dtheta = Math::XM_2PI / sliceCount;
		for (int j = 0; j <= sliceCount; j++)
		{
			const float c = cosf(j * dtheta);
			const float s = sinf(j * dtheta);

			Mesh::Vertex vertex;

			// Pozitia
			Math::Vector3 position;
			position.SetX(r * c);
			position.SetY(y);
			position.SetZ(r * s);

			// Coordonata de textura
			vertex.texC.x = (float)j / sliceCount;
			vertex.texC.y = 1.f - (float)i / stackCount;

			// Tangenta, normala, bitangenta
			Vector3 tangenta(-s, 0.f, c);
			Vector3 bitangenta(dr * c, -height, dr * s);
			Vector3 normal = tangenta % bitangenta;

			// Le salvam
			DirectX::XMStoreFloat3(&vertex.position, position);
			DirectX::XMStoreFloat4(&vertex.color, Vector4(j * dtheta < Math::XM_PI ? 1.f : 0.f, 1.f, 1.f, 1.f));
			DirectX::XMStoreFloat3(&vertex.normal, normal);
			DirectX::XMStoreFloat3(&vertex.tangent, tangenta);

			vertices.push_back(vertex);
		}
	}

	// Generare indecsi suprafata
	const int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < ringCount - 1; i++)
	{
		for (int j = 0; j < sliceCount; j++)
		{
			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);

			indices.push_back(i * ringVertexCount + j);
			indices.push_back((i + 1) * ringVertexCount + j + 1);
			indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	const Mesh::Index baseCenterIndex = (std::uint32_t)vertices.size();
	const Mesh::Index topCenterIndex = (std::uint32_t)vertices.size() + 1;
	const Mesh::Index topOffset = (ringCount - 1) * ringVertexCount;

	vertices.push_back({DirectX::XMFLOAT3{0.f, -height / 2.f, 0.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}});
	vertices.push_back({DirectX::XMFLOAT3{0.f, height / 2.f, 0.f}, DirectX::XMFLOAT4{1.f, 1.f, 1.f, 1.f}});

	for (int i = 0; i < sliceCount; i++)
	{
		indices.push_back(baseCenterIndex);
		indices.push_back(i);
		indices.push_back(i + 1);

		indices.push_back(topCenterIndex);
		indices.push_back(topOffset + i + 1);
		indices.push_back(topOffset + i);
	}

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateCube(float sideLength)
{
	using namespace Math;

	Mesh::Ptr mesh;
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	float halfSide = sideLength / 2.0f;

	// Fata frontala
	vertices.push_back({{-halfSide, -halfSide, -halfSide}, {1.f, 0.f, 0.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{-halfSide, halfSide, -halfSide}, {1.f, 0.f, 0.f, 1.f}, {0.f, 1.f}});
	vertices.push_back({{halfSide, halfSide, -halfSide}, {1.f, 0.f, 0.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{halfSide, -halfSide, -halfSide}, {1.f, 0.f, 0.f, 1.f}, {1.f, 0.f}});

	// Fata din spate
	vertices.push_back({{-halfSide, -halfSide, halfSide}, {0.f, 1.f, 0.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{halfSide, -halfSide, halfSide}, {0.f, 1.f, 0.f, 1.f}, {1.f, 0.f}});
	vertices.push_back({{halfSide, halfSide, halfSide}, {0.f, 1.f, 0.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{-halfSide, halfSide, halfSide}, {0.f, 1.f, 0.f, 1.f}, {0.f, 1.f}});

	// Fata stanga
	vertices.push_back({{-halfSide, -halfSide, halfSide}, {0.f, 0.f, 1.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{-halfSide, halfSide, halfSide}, {0.f, 0.f, 1.f, 1.f}, {0.f, 1.f}});
	vertices.push_back({{-halfSide, halfSide, -halfSide}, {0.f, 0.f, 1.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{-halfSide, -halfSide, -halfSide}, {0.f, 0.f, 1.f, 1.f}, {1.f, 0.f}});

	// Fata dreapta
	vertices.push_back({{halfSide, -halfSide, -halfSide}, {1.f, 1.f, 0.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{halfSide, halfSide, -halfSide}, {1.f, 1.f, 0.f, 1.f}, {0.f, 1.f}});
	vertices.push_back({{halfSide, halfSide, halfSide}, {1.f, 1.f, 0.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{halfSide, -halfSide, halfSide}, {1.f, 1.f, 0.f, 1.f}, {1.f, 0.f}});

	// Fata sus
	vertices.push_back({{-halfSide, halfSide, -halfSide}, {1.f, 0.f, 1.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{-halfSide, halfSide, halfSide}, {1.f, 0.f, 1.f, 1.f}, {0.f, 1.f}});
	vertices.push_back({{halfSide, halfSide, halfSide}, {1.f, 0.f, 1.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{halfSide, halfSide, -halfSide}, {1.f, 0.f, 1.f, 1.f}, {1.f, 0.f}});

	// Fata jos
	vertices.push_back({{-halfSide, -halfSide, -halfSide}, {0.f, 1.f, 1.f, 1.f}, {0.f, 0.f}});
	vertices.push_back({{halfSide, -halfSide, -halfSide}, {0.f, 1.f, 1.f, 1.f}, {1.f, 0.f}});
	vertices.push_back({{halfSide, -halfSide, halfSide}, {0.f, 1.f, 1.f, 1.f}, {1.f, 1.f}});
	vertices.push_back({{-halfSide, -halfSide, halfSide}, {0.f, 1.f, 1.f, 1.f}, {0.f, 1.f}});

	// Indices
	for (int i = 0; i < 24; i += 4)
	{
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);

		indices.push_back(i);
		indices.push_back(i + 2);
		indices.push_back(i + 3);
	}

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateSimpleCube(float sideLength)
{
	using namespace Math;

	Mesh::Ptr mesh;
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	float halfSide = sideLength / 2.0f;

	vertices = {
		{XMFLOAT3(-halfSide, -halfSide, halfSide), XMFLOAT4(1.f, 0.f, 0.f, 1.f)},
		{XMFLOAT3(halfSide, -halfSide, halfSide), XMFLOAT4(1.f, 0.f, 0.f, 1.f)},
		{XMFLOAT3(-halfSide, halfSide, halfSide), XMFLOAT4(1.f, 0.f, 0.f, 1.f)},
		{XMFLOAT3(halfSide, halfSide, halfSide), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
		{XMFLOAT3(-halfSide, -halfSide, -halfSide), XMFLOAT4(1.f, 0.f, 0.f, 1.f)},
		{XMFLOAT3(halfSide, -halfSide, -halfSide), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
		{XMFLOAT3(-halfSide, halfSide, -halfSide), XMFLOAT4(1.f, 1.f, 1.f, 1.f)},
		{XMFLOAT3(halfSide, halfSide, -halfSide), XMFLOAT4(1.f, 1.f, 1.f, 1.f)}};

	indices = {0, 1, 2, 1, 3, 2, 2, 3, 7, 2, 7, 6, 1, 7, 3, 1, 5, 7,
			   6, 7, 4, 7, 5, 4, 0, 4, 1, 1, 4, 5, 2, 6, 4, 0, 2, 4};

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateSimpleQuad(float sideLength)
{
	using namespace Math;

	Mesh::Ptr mesh;
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	float halfSide = sideLength / 2.0f;

	vertices = {
		{XMFLOAT3(-halfSide, halfSide, 0), XMFLOAT4(1.f, 0.f, 0.f, 1.f), XMFLOAT2(0, 0)},
		{XMFLOAT3(halfSide, halfSide, 0), XMFLOAT4(1.f, 0.f, 0.f, 1.f), XMFLOAT2(1, 0)},
		{XMFLOAT3(-halfSide, -halfSide, 0), XMFLOAT4(1.f, 0.f, 0.f, 1.f), XMFLOAT2(0, 1)},
		{XMFLOAT3(halfSide, -halfSide, 0), XMFLOAT4(1.f, 0.f, 0.f, 1.f), XMFLOAT2(1, 1)}};

	indices = {0, 2, 1, 1, 2, 3};

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateGrid(float width, float length, int columns, int rows)
{
	using namespace Math;

	Mesh::Ptr mesh;
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	const float dw = width / columns;
	const float dl = length / rows;

	for (int i = 0; i <= rows; i++)
	{
		const float currentLength = i * dl - length / 2.0f;

		for (int j = 0; j <= columns; j++)
		{
			Mesh::Vertex vertex;

			float x = j * dw - width / 2.0f;
			float z = currentLength;

			// Pozitie
			DirectX::XMStoreFloat3(&vertex.position, Vector3(x, 0.f, z));

			// Culoare
			DirectX::XMStoreFloat4(&vertex.color, Vector4(0.f, 0.f, 1.f, 1.f));

			// Textura
			vertex.texC.x = j / (float)columns;
			vertex.texC.y = i / (float)rows;

			// Tangenta
			DirectX::XMStoreFloat3(&vertex.tangent, Vector3(1.f, 0.f, 0.f));

			// Normala - pentru o suprafata plana orizontala, normala este in sus.
			DirectX::XMStoreFloat3(&vertex.normal, Vector3(0.f, 1.f, 0.f));

			vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			indices.push_back(j + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + i * (rows + 1));
			indices.push_back(j + i * (rows + 1));

			indices.push_back(j + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + i * (rows + 1));
		}
	}

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

// Incompleta
Mesh::Ptr GeometryGenerator::GenerateGeoSphere(float radius, const Math::Vector3 center, int nrOfSubDivisions)
{
	using namespace Math;

	Mesh::Ptr mesh;
	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	float t = (1.0f + sqrt(5.0f)) / 2.0f;

	std::vector<Math::Vector3> positions = {
		{-1, t, 0},
		{1, t, 0},
		{-1, -t, 0},
		{1, -t, 0},
		{0, -1, t},
		{0, 1, t},
		{0, -1, -t},
		{0, 1, -t},
		{t, 0, -1},
		{t, 0, 1},
		{-t, 0, -1},
		{-t, 0, 1},
	};

	for (auto& position : positions)
	{
		position.Normalize();
	}

	indices = {
		0, 11, 5, 0, 5, 1, 0, 1, 7, 0, 7, 10, 0, 10, 11, 1, 5, 9, 5, 11, 4,  11, 10, 2,  10, 7, 6, 7, 1, 8,
		3, 9,  4, 3, 4, 2, 3, 2, 6, 3, 6, 8,  3, 8,  9,  4, 9, 5, 2, 4,  11, 6,  2,  10, 8,  6, 7, 9, 8, 1,
	};

	for (const auto& position : positions)
	{
		Mesh::Vertex vertex;

		XMStoreFloat3(&vertex.position, position);
				XMStoreFloat4(&vertex.color, DirectX::XMVectorSet((float)position.GetX(), 0.5f, (float)position.GetY(), 1.0f));

		vertices.push_back(vertex);
	}

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::Subdivide(mesh, nrOfSubDivisions);
	GeometryHelper::ProjectVerticesOntoSphere(mesh, radius);
	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);
	GeometryHelper::MoveVerticesToPosition(mesh, center);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateTriangle()
{
	using namespace Math;

	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	vertices = {XMFLOAT3{0.f, 1.f, 0.f}, XMFLOAT3{0.866f, -0.5f, 0}, XMFLOAT3{-0.866f, -0.5f, 0}};
	indices = {0, 1, 2};

	return Mesh::Ptr(new Mesh(vertices, indices));
}

#ifdef USE_BEZIER
Mesh::Ptr GeometryGenerator::GenerateTerrainWithBezierCurves(float width, float length, int subdivideCount)
{
	using namespace Math;

	Mesh::Ptr mesh = GenerateGrid(width, length, 2, 2);

	std::vector<std::vector<Vector3>> m = {
		{Vector3(-width / 2, 0, -length / 2), Vector3(0, -20, -length / 2), Vector3(width / 2, 0, -length / 2)},
		{Vector3(-width / 2, 25, length / 2), Vector3(0, -20, length / 2), Vector3(width / 2, 0, length / 2)}};

	std::vector<Mesh::Vertex> vertices;
	std::vector<UINT> indices;

	int columns = 200;
	int rows = 200;

	const float dw = width / columns;
	const float dl = length / rows;

	for (int i = 0; i <= rows; i++)
	{
		const float currentLength = i * dl - length / 2.0f;

		for (int j = 0; j <= columns; j++)
		{
			Mesh::Vertex vertex;

			Vector3 position = CubicBezier(m, j * 1.0f / columns, i * 1.0f / rows);

			// Pozitie
			DirectX::XMStoreFloat3(&vertex.position, position);

			// Textura
			vertex.texC.x = j / (float)columns;
			vertex.texC.y = i / (float)rows;

			// Tangenta
			DirectX::XMStoreFloat3(&vertex.tangent, Vector3(1.f, 0.f, 0.f));

			// Normala - pentru o suprafata plana orizontala, normala este in sus.
			DirectX::XMStoreFloat3(&vertex.normal, Vector3(0.f, 1.f, 0.f));

			vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < columns; j++)
		{
			indices.push_back(j + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + i * (rows + 1));
			indices.push_back(j + i * (rows + 1));

			indices.push_back(j + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + (i + 1) * (rows + 1));
			indices.push_back(j + 1 + i * (rows + 1));
		}
	}

	mesh = std::make_shared<Mesh>(vertices, indices);

	GeometryHelper::ComputeVertexNormals(mesh);

	return mesh;
}
#endif

Mesh::Ptr GeometryGenerator::GenerateChunk(
	const Math::Vector3& position,
	float chunkWidth,
	float chunkLength,
	float terrainWidth,
	float terrainLength,
	std::function<float(float, float)> heightFunction,
	int subdivideCount)
{
	using namespace Math;

	Mesh::Ptr mesh = GenerateGrid(chunkWidth, chunkLength, 2, 2);

	GeometryHelper::Subdivide(mesh, subdivideCount);
	GeometryHelper::MoveVerticesToPosition(mesh, position);
	GeometryHelper::TransformTextureCoordinates(mesh, terrainWidth, terrainLength);
	GeometryHelper::ApplyHeightFunctionForGrid(mesh, heightFunction);
	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}

Mesh::Ptr GeometryGenerator::GenerateChunks(
	std::vector<Math::AABB>& aabbs,
	std::vector<SubMesh>& submeshs,
	std::function<float(float, float)> heightFunction,
	const float gridWidth,
	const float gridLength,
	const int chunkKernelSize,
	const int chunkCountPerSide)
{
	using namespace Math;

	const int sidePointCount =
		2 * chunkKernelSize + chunkCountPerSide - 3 + (chunkKernelSize - 2) * (chunkCountPerSide - 2);

	const float dz = gridWidth / sidePointCount;
	const float dx = gridLength / sidePointCount;

	std::vector<Mesh::Vertex> vertices;
	std::vector<Mesh::Index> indices;

	for (int i = 0; i < sidePointCount; i++)
	{
		const float currentX = i * dx - gridLength / 2.0f;

		for (int j = 0; j < sidePointCount; j++)
		{
			Mesh::Vertex vertex;

			float x = currentX;
			float z = j * dz - gridWidth / 2.0f;
			float y = heightFunction(x, z);

			// Pozitie
			DirectX::XMStoreFloat3(&vertex.position, Vector3(x, y, z));

			// Culoare
			DirectX::XMStoreFloat4(&vertex.color, Vector4(0.f, 0.f, 1.f, 1.f));

			// Textura
			vertex.texC.x = j / (float)sidePointCount;
			vertex.texC.y = i / (float)sidePointCount;

			// Tangenta
			DirectX::XMStoreFloat3(&vertex.tangent, Vector3(1.f, 0.f, 0.f));

			// Normala - pentru o suprafata plana orizontala, normala este in sus.
			DirectX::XMStoreFloat3(&vertex.normal, Vector3(0.f, 1.f, 0.f));

			vertices.push_back(vertex);
		}
	}

	for (int i = 0; i < chunkCountPerSide; i++)
	{
		for (int j = 0; j < chunkCountPerSide; j++)
		{
			int startVertexPosition = (chunkKernelSize - 1) * j + (chunkKernelSize - 1) * i * sidePointCount;

			for (int k = startVertexPosition; k < startVertexPosition + chunkKernelSize - 1; k++)
			{
				for (int l = 0; l < chunkKernelSize - 1; l++)
				{
					indices.push_back(k + l * sidePointCount);
					indices.push_back(k + 1 + (l + 1) * sidePointCount);
					indices.push_back(k + (l + 1) * sidePointCount);

					indices.push_back(k + l * sidePointCount);
					indices.push_back(k + 1 + l * sidePointCount);
					indices.push_back(k + 1 + (l + 1) * sidePointCount);
				}
			}

			SubMesh subMesh;
			subMesh.baseVertexLocation = 0;
			subMesh.indexCount = 6 * (size_t)pow(chunkKernelSize - 1, 2);
			subMesh.startIndexLocation = indices.size() - subMesh.indexCount;

			Math::AABB aabb;
			for (int i = 0; i < subMesh.indexCount; i++)
			{
				aabb.EnlargeForPoint(vertices[indices[i + subMesh.startIndexLocation]].position);
			}

			submeshs.push_back(subMesh);
			aabbs.push_back(aabb);
		}
	}

	Mesh::Ptr mesh = Mesh::Ptr(new Mesh(vertices, indices));

	GeometryHelper::ComputeVertexNormalsAndTangents(mesh);

	return mesh;
}
