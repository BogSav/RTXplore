#pragma once

#include "FrameResources.hpp"
#include "GeometryRenderer.hpp"
#include "Object.hpp"

#include <unordered_map>

class ObjectRenderer : public GeometryRenderer
{
public:
	using Ptr = std::unique_ptr<ObjectRenderer>;

	static ObjectRenderer::Ptr CreateObjectRenderer(misc::render_descriptors::DX_OBJECTS_RENDERER_DESCRIPTOR descriptor);

	void Render(misc::rasterization::RenderLayer::Value renderLayer) const override;
	void BuildAccelerationStructures() override;
	void FrustumCulling(const CameraController& cameraController) override;
	void Update(float delatTime) override;

	inline const Math::AABB& GetAABB(std::string name) const { return m_boundingBoxes.at(name); }
	inline const SubMesh& GetSubMesh(std::string name) const { return m_subMeshes.at(name); }
	inline const Object::Vec& GetObjects() const { return m_objects; }

	~ObjectRenderer();

private:
	ObjectRenderer() = default;

	void LoadGeometry(DescriptorVariant descriptor) override;
	void CreateObjects(const misc::render_descriptors::DX_OBJECTS_RENDERER_DESCRIPTOR&);

	GraphicsPSO::Ptr m_shadowDebugPSO;
	Texture::Ptr m_shadowTexture;

	std::unordered_map<std::string, Math::AABB> m_boundingBoxes;
	std::unordered_map<std::string, SubMesh> m_subMeshes;

	std::vector<Object::Ptr> m_objects;
};