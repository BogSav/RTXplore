#pragma once

#include "HlslUtils.h"
#include "engine/math/Vector.hpp"

class MaterialManager
{
public:
	struct ShlickCoefficients
	{
		static engine::math::Vector3 Water;
		static engine::math::Vector3 Glass;
		static engine::math::Vector3 Plastic;
		static engine::math::Vector3 Gold;
		static engine::math::Vector3 Silver;
		static engine::math::Vector3 Copper;
	};

	class Material
	{
	public:
		Material(std::string name, UINT materialCB_ID);
		void SetMaterialProperties(const MaterialProperties& materialProperties);

		const MaterialProperties& GetMaterialProperties() const { return m_materialProperties; }
		const UINT& GetMaterialCB_ID() const { return m_materialCB_ID; }
		const std::string& GetName() const { return m_name; }

	private:
		const std::string m_name;
		const UINT m_materialCB_ID;

		MaterialProperties m_materialProperties;
	};

	MaterialManager() = default;
	void LoadDefaultMaterials();

	const MaterialProperties& GetMaterialConstantBuffer(std::string name) const;
	const UINT& GetMaterialCB_ID(std::string name) const;
	const Material& GetMaterial(std::string name) const;

	const std::vector<Material>& GetMaterials() const { return m_materials; }

private:
	std::vector<Material> m_materials;
};