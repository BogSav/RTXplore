#pragma once

#include "Utilities.hpp"
#include "engine/math/Vector.hpp"

namespace engine::gfx
{

class LightSource
{
public:
	using Vec = std::vector<LightSource>;

	enum class Type
	{
		NONE,
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT
	};

public:
	LightSource() = delete;
	LightSource(Type type = Type::NONE);

	inline size_t GetID() const { return m_id; }
	inline Type GetType() const { return m_type; }

public:
	LightProperties m_lightProperties = {};

private:
	static size_t nrOfDirectionalLights;
	static size_t nrOfPointLights;
	static size_t nrOfSpotLights;
 
	Type m_type;
	size_t m_id;
};

}  // namespace engine::gfx
