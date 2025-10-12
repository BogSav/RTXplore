#include "LightSource.hpp"

#include "engine/core/Settings.hpp"

size_t LightSource::nrOfDirectionalLights = 0;
size_t LightSource::nrOfPointLights = 0;
size_t LightSource::nrOfSpotLights = 0;

LightSource::LightSource(Type type) : m_type(type)
{
	switch (m_type)
	{
	case LightSource::Type::NONE: throw engine::core::CustomException("Sursa de lumina nu trebuei sa fie None"); break;
	case LightSource::Type::DIRECTIONAL_LIGHT: m_id = nrOfDirectionalLights++; break;
	case LightSource::Type::POINT_LIGHT:
		m_id = engine::core::Settings::GetGameSettings().GetMaxNumberOfDirectionalLights() + nrOfPointLights++;
		break;
	case LightSource::Type::SPOT_LIGHT:
		m_id = engine::core::Settings::GetGameSettings().GetMaxNumberOfDirectionalLights()
			+ engine::core::Settings::GetGameSettings().GetMaxNumberOfPointLights() + nrOfSpotLights++;
		break;
	default: throw engine::core::CustomException("Type of LightSource not supported"); break;
	}

	assert(nrOfDirectionalLights <= engine::core::Settings::GetGameSettings().GetMaxNumberOfDirectionalLights());
	assert(nrOfSpotLights <= engine::core::Settings::GetGameSettings().GetMaxNumberOfSpotLights());
	assert(nrOfPointLights <= engine::core::Settings::GetGameSettings().GetMaxNumberOfPointLights());
	assert(m_id <= engine::core::Settings::GetMaxNrOfLightSources());
}
