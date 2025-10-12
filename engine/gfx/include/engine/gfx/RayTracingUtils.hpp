#pragma once

namespace RayTracingInstanceMasks
{
enum Value : int
{
	Terrain = 0b00000001,
	Water = 0b00000010,
	Object = 0b00000100
};
}