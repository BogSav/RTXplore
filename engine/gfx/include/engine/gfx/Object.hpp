#pragma once

#include "Utilities.hpp"
#include "GraphicsResources.hpp"
#include "Mesh.hpp"

class Object
{
public:
	using Ptr = std::shared_ptr<Object>;
	using Vec = std::vector<std::shared_ptr<Object>>;

	Object(const engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR&, const SubMesh& subMesh, const engine::math::AABB& aabb);
	Object(const engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR&);
	Object() = delete;

	virtual void Render(engine::gfx::rasterization::RenderLayer::Value renderLayer);
	virtual void Update(float delatTime);

	const engine::math::Matrix4& GetTextureTransform() const { return m_textureTransform; }
	const engine::math::Quaternion& GetRotation() const { return m_rotation; }
	const engine::math::Matrix4& GetTransform() const { return m_transform; }
	const engine::math::Vector3& GetPosition() const { return m_position; }
	const engine::math::Vector3& GetScale() const { return m_scale; }
	const engine::math::Vector4& GetColor() const { return m_color; }

	inline const engine::math::AABB& GetObjectSpaceAABB() const { return m_objectSpaceAABB; }
	inline const engine::math::AABB& GetWorldSpaceAABB() const { return m_worldSpaceAABB; }
	inline const SubMesh& GetSubMesh() const { return m_subMesh; }

	inline bool IsDirty() const { return m_dirtyCount != 0 && !m_isStatic; }
	inline UINT GetMaterialCB_ID() const { return m_materialCB_ID; }
	inline UINT GetObjectCB_ID() const { return m_objectCB_ID; }
	inline bool IsVisible() const { return m_isVisible; }
	inline bool IsStatic() const { return m_isStatic; }

	void SetTextureTransform(const engine::math::Matrix4& toSet);
	void SetRotation(const engine::math::Quaternion& toSet);
	void SetTransform(const engine::math::Matrix4& toSet);
	void SetPosition(const engine::math::Vector3& toSet);
	void SetScale(const engine::math::Vector3& toSet);
	void SeColor(const engine::math::Vector4& toSet);
	void SetObjectCB_ID(UINT toSet);
	void SetVisible(bool toSet);

	void DecreaseDirtyCount();
	void SetDirty();

protected:
	engine::math::AABB m_objectSpaceAABB;
	engine::math::AABB m_worldSpaceAABB;

private:
	engine::math::Quaternion m_rotation;
	engine::math::Vector3 m_position;
	engine::math::Vector3 m_scale;
	engine::math::Vector4 m_color;

	engine::math::Matrix4 m_textureTransform;
	engine::math::Matrix4 m_transform;

	SubMesh m_subMesh;

	bool m_isStatic;
	bool m_isVisible;
	int m_dirtyCount;

	UINT m_objectCB_ID;
	UINT m_materialCB_ID;
};