#include "MaterialManager.hpp"

#include "FrameResources.hpp"

namespace engine::gfx
{

engine::math::Vector3 MaterialManager::ShlickCoefficients::Water = engine::math::Vector3(0.02, 0.02, 0.02);
engine::math::Vector3 MaterialManager::ShlickCoefficients::Glass = engine::math::Vector3(0.08, 0.08, 0.08);
engine::math::Vector3 MaterialManager::ShlickCoefficients::Plastic = engine::math::Vector3(0.05, 0.05, 0.05);
engine::math::Vector3 MaterialManager::ShlickCoefficients::Gold = engine::math::Vector3(1.0, 0.71, 0.29);
engine::math::Vector3 MaterialManager::ShlickCoefficients::Silver = engine::math::Vector3(0.95, 0.93, 0.88);
engine::math::Vector3 MaterialManager::ShlickCoefficients::Copper = engine::math::Vector3(0.95, 0.64, 0.54);

void MaterialManager::LoadDefaultMaterials()
{
	engine::math::Vector3 identityVector = engine::math::Vector3(engine::math::kIdentity);
	engine::math::Vector3 nullVector = engine::math::Vector3(engine::math::kZero);

	MaterialProperties materialProps;
	materialProps.illumType = 0;

	// Default material, something basic
	{
		materialProps.Ka = engine::math::Vector3(0.02, 0.02, 0.02);
		materialProps.Kd = engine::math::Vector3(1, 0, 0);
		materialProps.Ks = identityVector;

		materialProps.Transparency = 0;
		materialProps.IndexOfRefraction = 0;
		materialProps.Shiness = 0.2;
		materialProps.Reflectivity = 0;

		materialProps.Rf0 = nullVector;

		Material material("Default", FrameResources::GetMaterialCB_ID());
		material.SetMaterialProperties(materialProps);

		m_materials.push_back(material);
	}

	// Water material
	{
		materialProps.Transparency = 0.4;
		materialProps.IndexOfRefraction = 1.33;
		materialProps.Reflectivity = 0.8;

		materialProps.Rf0 = ShlickCoefficients::Water;

		Material material("Water", FrameResources::GetMaterialCB_ID());
		material.SetMaterialProperties(materialProps);

		m_materials.push_back(material);
	}
}

const MaterialManager::Material& MaterialManager::GetMaterial(std::string name) const
{
	return *std::find_if(
		m_materials.cbegin(),
		m_materials.cend(),
		[&name](const Material& material) { return material.GetName() == name; });
}

const UINT& MaterialManager::GetMaterialCB_ID(std::string name) const
{
	return GetMaterial(name).GetMaterialCB_ID();
}

const MaterialProperties& MaterialManager::GetMaterialConstantBuffer(std::string name) const
{
	return GetMaterial(name).GetMaterialProperties();
}

MaterialManager::Material::Material(std::string name, UINT materialCB_ID) : m_name(name), m_materialCB_ID(materialCB_ID)
{
}

void MaterialManager::Material::SetMaterialProperties(const MaterialProperties& materialProperties)
{
	m_materialProperties = materialProperties;
}

}  // namespace engine::gfx
