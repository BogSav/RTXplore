#include "Object.hpp"

#include "GeometryGenerator.hpp"

namespace engine::gfx
{

using namespace engine::gfx::rasterization;
using namespace engine::gfx::render_descriptors;
using namespace engine::gfx::RSBinding;

Object::Object(
	const engine::gfx::render_descriptors::DX_OBJECT_DESCRIPTOR& descriptor, const SubMesh& subMesh, const engine::math::AABB& aabb)
	: Object(descriptor)
{
	m_objectSpaceAABB = aabb;
	m_subMesh = subMesh;

	if (m_isStatic)
	{
		m_worldSpaceAABB = m_objectSpaceAABB.Transformed(m_transform);
	}
}

Object::Object(const DX_OBJECT_DESCRIPTOR& descriptor)
{
	m_objectCB_ID = descriptor.objectCB_ID;
	m_materialCB_ID = descriptor.materialCB_ID;

	m_scale = descriptor.scale;
	m_rotation = descriptor.rotation;
	m_color = descriptor.color;
	m_position = descriptor.position;

	m_isStatic = descriptor.isStatic;

	if (m_isStatic)
	{
		m_transform = engine::math::Matrix4::MakeScale(m_scale) * engine::math::Matrix4::MakeMatrixRotationQuaternion(m_rotation)
			* engine::math::Matrix4::MakeTranslation(m_position);

		m_dirtyCount = 0;
	}
	else
	{
		SetDirty();
	}

	m_textureTransform = descriptor.textureTransform;
}

void Object::Render(engine::gfx::rasterization::RenderLayer::Value renderLayer)
{
	GraphicsContext& graphicsContext = GraphicsResources::GetInstance().GetGraphicsContext();
	FrameResources& frameResources = GraphicsResources::GetInstance().GetFrameResources();

	graphicsContext.SetConstantBuffer(
		DefaultRSBindings::ObjectCB, frameResources.m_perObjectCB.GetGpuVirtualAdress(m_objectCB_ID));

	// if (UseLighting())
	//{
	//	pCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	// pCommandList->SetGraphicsRootConstantBufferView(
	//	DefaultRSBindings::MaterialCB, pFrameResources->m_perMaterialCB.GetGpuVirtualAdress(GetMaterialCB_ID()));

	// pCommandList->SetGraphicsRootDescriptorTable(DefaultRSBindings::DiffuseTexture, m_diffuseTextureHandle);
	// pCommandList->SetGraphicsRootDescriptorTable(DefaultRSBindings::NormalTexture, m_normalTextureHandle);
	// pCommandList->SetGraphicsRootDescriptorTable(DefaultRSBindings::DisplacementTexture,
	// m_displacementTextureHandle);

	// if (UseShadows())
	//{
	//	pCommandList->SetGraphicsRootDescriptorTable(1, m_shadowTextureHandle);
	// }
	//}
	graphicsContext.DrawIndexed(
		(UINT)m_subMesh.indexCount, (UINT)m_subMesh.startIndexLocation, (UINT)m_subMesh.baseVertexLocation);
}

// Recalculate the transformation matrix
void Object::Update(float deltaTime)
{
	if (m_isStatic)
		return;

	m_transform = engine::math::Matrix4::MakeMatrixRotationQuaternion(m_rotation) * engine::math::Matrix4::MakeScale(m_scale)
		* engine::math::Matrix4::MakeTranslation(m_position);
	m_worldSpaceAABB = m_objectSpaceAABB.Transformed(m_transform);

	SetDirty();
}

void Object::SetTransform(const engine::math::Matrix4& toSet)
{
	assert(!m_isStatic);

	m_transform = toSet;
	SetDirty();
}

void Object::SetTextureTransform(const engine::math::Matrix4& toSet)
{
	m_textureTransform = toSet;
	SetDirty();
}

void Object::SetPosition(const engine::math::Vector3& toSet)
{
	assert(!m_isStatic);

	m_position = toSet;
	SetDirty();
}

void Object::SetScale(const engine::math::Vector3& toSet)
{
	assert(!m_isStatic);

	m_scale = toSet;
	SetDirty();
}

void Object::SetRotation(const engine::math::Quaternion& toSet)
{
	assert(!m_isStatic);

	m_rotation = toSet;
	SetDirty();
}

void Object::SeColor(const engine::math::Vector4& toSet)
{
	assert(!m_isStatic);

	m_color = toSet;
	SetDirty();
}

void Object::SetVisible(bool toSet)
{
	m_isVisible = toSet;
}

void Object::SetObjectCB_ID(UINT toSet)
{
	m_objectCB_ID = toSet;
}

void Object::SetDirty()
{
	m_dirtyCount = engine::core::Settings::GetFrameResourcesCount();
}

void Object::DecreaseDirtyCount()
{
	m_dirtyCount--;
	assert(m_dirtyCount >= 0);
}

}  // namespace engine::gfx
