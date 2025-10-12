#pragma once

#include "Utilities.hpp"
#include "GraphicsResources.hpp"
#include "Mesh.hpp"

class Object
{
public:
	using Ptr = std::shared_ptr<Object>;
	using Vec = std::vector<std::shared_ptr<Object>>;

	Object(const misc::render_descriptors::DX_OBJECT_DESCRIPTOR&, const SubMesh& subMesh, const Math::AABB& aabb);
	Object(const misc::render_descriptors::DX_OBJECT_DESCRIPTOR&);
	Object() = delete;

	virtual void Render(misc::rasterization::RenderLayer::Value renderLayer);
	virtual void Update(float delatTime);

	const Math::Matrix4& GetTextureTransform() const { return m_textureTransform; }
	const Math::Quaternion& GetRotation() const { return m_rotation; }
	const Math::Matrix4& GetTransform() const { return m_transform; }
	const Math::Vector3& GetPosition() const { return m_position; }
	const Math::Vector3& GetScale() const { return m_scale; }
	const Math::Vector4& GetColor() const { return m_color; }

	inline const Math::AABB& GetObjectSpaceAABB() const { return m_objectSpaceAABB; }
	inline const Math::AABB& GetWorldSpaceAABB() const { return m_worldSpaceAABB; }
	inline const SubMesh& GetSubMesh() const { return m_subMesh; }

	inline bool IsDirty() const { return m_dirtyCount != 0 && !m_isStatic; }
	inline UINT GetMaterialCB_ID() const { return m_materialCB_ID; }
	inline UINT GetObjectCB_ID() const { return m_objectCB_ID; }
	inline bool IsVisible() const { return m_isVisible; }
	inline bool IsStatic() const { return m_isStatic; }

	void SetTextureTransform(const Math::Matrix4& toSet);
	void SetRotation(const Math::Quaternion& toSet);
	void SetTransform(const Math::Matrix4& toSet);
	void SetPosition(const Math::Vector3& toSet);
	void SetScale(const Math::Vector3& toSet);
	void SeColor(const Math::Vector4& toSet);
	void SetObjectCB_ID(UINT toSet);
	void SetVisible(bool toSet);

	void DecreaseDirtyCount();
	void SetDirty();

protected:
	Math::AABB m_objectSpaceAABB;
	Math::AABB m_worldSpaceAABB;

private:
	Math::Quaternion m_rotation;
	Math::Vector3 m_position;
	Math::Vector3 m_scale;
	Math::Vector4 m_color;

	Math::Matrix4 m_textureTransform;
	Math::Matrix4 m_transform;

	SubMesh m_subMesh;

	bool m_isStatic;
	bool m_isVisible;
	int m_dirtyCount;

	UINT m_objectCB_ID;
	UINT m_materialCB_ID;
};