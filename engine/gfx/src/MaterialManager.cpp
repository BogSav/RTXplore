#include "MaterialManager.hpp"

#include "DX_FrameResources.hpp"

Math::Vector3 MaterialManager::ShlickCoefficients::Water = Math::Vector3(0.02, 0.02, 0.02);
Math::Vector3 MaterialManager::ShlickCoefficients::Glass = Math::Vector3(0.08, 0.08, 0.08);
Math::Vector3 MaterialManager::ShlickCoefficients::Plastic = Math::Vector3(0.05, 0.05, 0.05);
Math::Vector3 MaterialManager::ShlickCoefficients::Gold = Math::Vector3(1.0, 0.71, 0.29);
Math::Vector3 MaterialManager::ShlickCoefficients::Silver = Math::Vector3(0.95, 0.93, 0.88);
Math::Vector3 MaterialManager::ShlickCoefficients::Copper = Math::Vector3(0.95, 0.64, 0.54);

void MaterialManager::LoadDefaultMaterials()
{
	Math::Vector3 identityVector = Math::Vector3(Math::kIdentity);
	Math::Vector3 nullVector = Math::Vector3(Math::kZero);

	MaterialProperties materialProps;
	materialProps.illumType = 0;

	// Default material, something basic
	{
		materialProps.Ka = Math::Vector3(0.02, 0.02, 0.02);
		materialProps.Kd = Math::Vector3(1, 0, 0);
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
