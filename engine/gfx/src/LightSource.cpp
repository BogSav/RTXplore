#include "LightSource.hpp"

#include "engine/core/Settings.hpp"

size_t LightSource::nrOfDirectionalLights = 0;
size_t LightSource::nrOfPointLights = 0;
size_t LightSource::nrOfSpotLights = 0;

LightSource::LightSource(Type type) : m_type(type)
{
	switch (m_type)
	{
	case LightSource::Type::NONE: throw misc ::CustomException("Sursa de lumina nu trebuei sa fie None"); break;
	case LightSource::Type::DIRECTIONAL_LIGHT: m_id = nrOfDirectionalLights++; break;
	case LightSource::Type::POINT_LIGHT:
		m_id = Settings ::GetGameSettings().GetMaxNumberOfDirectionalLights() + nrOfPointLights++;
		break;
	case LightSource::Type::SPOT_LIGHT:
		m_id = Settings::GetGameSettings().GetMaxNumberOfDirectionalLights()
			+ Settings::GetGameSettings().GetMaxNumberOfPointLights() + nrOfSpotLights++;
		break;
	default: throw misc::CustomException("Type of LightSource not supported"); break;
	}

	assert(nrOfDirectionalLights <= Settings::GetGameSettings().GetMaxNumberOfDirectionalLights());
	assert(nrOfSpotLights <= Settings::GetGameSettings().GetMaxNumberOfSpotLights());
	assert(nrOfPointLights <= Settings::GetGameSettings().GetMaxNumberOfPointLights());
	assert(m_id <= Settings::GetMaxNrOfLightSources());
}
