#pragma once

#include "Mesh.hpp"

#include <unordered_map>
#include <functional>
#include <string>

namespace engine::gfx
{

struct GeometryHelper
{
	static void ComputeVertexNormalsAndTangents(Mesh::Ptr mesh);
	static void Subdivide(Mesh::Ptr mesh, int nrOfSubdivisions = 1);
	static void ProjectVerticesOntoSphere(Mesh::Ptr mesh, float radius);
	static void MoveVerticesToPosition(Mesh::Ptr mesh, engine::math::Vector3 position);
	static void ApplyHeightFunctionForGrid(Mesh::Ptr mesh, std::function<float(float, float)>);
	static void TransformTextureCoordinates(Mesh::Ptr mesh, float width, float length);
	static void ChnageColor(Mesh::Ptr mesh, engine::math::Vector4 color);
	static Mesh::Vertex GetInterpolatedVertex(const Mesh::Vertex& v0, const Mesh::Vertex& v1, float t = 0.5f);

private:
	static std::string GetEdgeKey(Mesh::Index a, Mesh::Index b);
	static Mesh::Index FindOrCreateMidpoint(
		Mesh::Index indexA,
		Mesh::Index indexB,
		std::unordered_map<std::string, Mesh::Index>& midpointCache,
		Mesh::Ptr mesh,
		std::vector<Mesh::Vertex>& newVertices);
};

}  // namespace engine::gfx
