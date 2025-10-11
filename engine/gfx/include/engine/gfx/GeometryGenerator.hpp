#pragma once

#include "GeometryHelper.hpp"

class GeometryGenerator
{
public:
	static Mesh::Ptr GenerateCylinder(
		const float bottomRadius,
		const float topRadius,
		const float height,
		const int stackCount,
		const int sliceCount);
	static Mesh::Ptr GenerateCube(float sideLength);
	static Mesh::Ptr GenerateSimpleCube(float sideLength);
	static Mesh::Ptr GenerateSimpleQuad(float sideLength);
	static Mesh::Ptr GenerateGrid(float width, float length, int columns, int rows);
	static Mesh::Ptr GenerateGeoSphere(float radius, const Math::Vector3 center, int nrOfSubDivisions = 0);
	static Mesh::Ptr GenerateTriangle();
	static Mesh::Ptr GenerateTerrainWithBezierCurves(float width, float length, int subdivideCount = 5);
	static Mesh::Ptr GenerateChunk(
		const Math::Vector3& position,
		float chunkWidth,
		float chunkLength,
		float terrainWidth,
		float terrainLength,
		std::function<float(float, float)> heightFunction,
		int subdivideCount = 5);
	static Mesh::Ptr GenerateChunks(
		std::vector<Math::AABB>& aabbs,
		std::vector<SubMesh>& submeshs,
		std::function<float(float, float)> heightFunction,
		const float gridWidth,
		const float gridLength,
		const int chunkKernelSize,
		const int chunkCountPerSide);
};