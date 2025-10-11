#pragma once

#ifdef HLSL
typedef float2 XMFLOAT2;
typedef float3 XMFLOAT3;
typedef float4 XMFLOAT4;
typedef float4 XMVECTOR;
typedef float4x4 XMMATRIX;
typedef float4x4 XMFLOAT4X4;
typedef uint UINT;
#else
#include "d3dx12.h"
#include <DirectXMath.h>
using namespace DirectX;
#endif

#ifndef DIR_LIGHTS_COUNT
#define DIR_LIGHTS_COUNT 0
#endif

#ifndef POINT_LIGHTS_COUNT
#define POINT_LIGHTS_COUNT 0
#endif

#ifndef SPOT_LIGHT_COUNT
#define SPOT_LIGHT_COUNT 0
#endif

#ifndef MAX_NUMBER_OF_LIGHTS
#define MAX_NUMBER_OF_LIGHTS 9
#endif

#ifndef WAVE_PROPERTIES_COUNT
#define WAVE_PROPERTIES_COUNT 2
#endif

#ifndef MAX_TESSELATION_FACTOR
#define MAX_TESSELATION_FACTOR 1.0
#endif

#ifndef MAX_RAY_RECURSION_DEPTH
#define MAX_RAY_RECURSION_DEPTH 2
#endif

static float inShadowRadiance = 0.35;

////////////////////////////////////////////////////////////////////////////////
// Structuri ajutatoare
////////////////////////////////////////////////////////////////////////////////
struct WaveProperties
{
	XMFLOAT2 direction;
	float wavelength;
	float amplitude;
	float speed;
	float steepness;

	XMFLOAT2 padding1;
};

struct LightProperties
{
	XMFLOAT3 Strength;
	float Kc;

	XMFLOAT3 Direction;
	float Kl;

	XMFLOAT3 Position;
	float SpotAngle;

	float Kq;
	XMFLOAT3 padding;

	XMFLOAT4X4 lightTransformMatrix;
};

struct MaterialProperties
{
	/// Ambient light reflexion coefficient
	XMFLOAT3 Ka;

	/// Specular exponent
	/// - values in range 0 - 1 which will be converted to 0 - 256
	/// - this is the shiness
	float Shiness;

	/// Diffuse light reflexion coefficient
	XMFLOAT3 Kd;

	/// Optical density
	/// - this is the index of refraction
	float IndexOfRefraction;

	/// Specular light reflexion
	XMFLOAT3 Ks;

	/// Dissolve variable
	/// - denots transparency
	/// - equal to (1 - opacity)
	/// - is how much a material disolves in the background
	float Transparency;

	/// Shlick coefficient
	/// - it's a material property
	XMFLOAT3 Rf0;

	/// Reflectivity
	/// - how much of the environemtn is reflected
	/// - this also determins how much the environment will be refracted by (1 - reflectivity)
	float Reflectivity;

	/// Illumination type
	/// - by type I mean the illumination model: Lamber, Phong...
	int illumType;  // Illumination type
	XMFLOAT3 padding;
};

////////////////////////////////////////////////////////////////////////////////
// Structuri pentru constant buffere (cb)
////////////////////////////////////////////////////////////////////////////////
struct PassConstantBuffer
{
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 invViewMatrix;
	XMFLOAT4X4 projMatrix;
	XMFLOAT4X4 invProjMatrix;
	XMFLOAT4X4 viewProjMatrix;
	XMFLOAT4X4 invViewProjMatrix;

	XMFLOAT3 eyePosition;
	float nearZ;

	XMFLOAT2 renderTargetSize;
	float farZ;
	float totalTime;

	XMFLOAT2 invRenderTargetSize;
	float deltaTime;
	UINT renderMode;

	XMFLOAT4 ambientLight;

	XMFLOAT3 fogColor;
	float fogStart;

	LightProperties lightSources[MAX_NUMBER_OF_LIGHTS];
};

struct ObjectConstantBuffer
{
	XMFLOAT4X4 worldMatrix;
	XMFLOAT4X4 invWorldMatrix;
	XMFLOAT4X4 textureTransform;
};

struct WaterConstantBuffer
{
	XMFLOAT3 cubeMapCenter;
	float cubeMapSphereRadius;
	XMFLOAT4 waterColor;
	WaveProperties waveParameters[WAVE_PROPERTIES_COUNT];
};


////////////////////////////////////////////////////////////////////////////////
// RayTracing dedicated enums
////////////////////////////////////////////////////////////////////////////////
namespace RayType
{
enum Value
{
	Radiance = 0,
	Shadow,
	Count
};
}

namespace RenderMode
{
enum Value : UINT
{
	Everything = 0b1111,
	BigWaves = 0b0001,
	SmallWaves = 0b0010
};
}

namespace TraceRayParameters
{
static const UINT InstanceMask = ~0;  // Everything is visible.

namespace HitGroup
{
static const UINT Offset[RayType::Count] = {
	0,  // Radiance ray
	1  // Shadow ray
};
}  // namespace HitGroup

namespace MissShader
{
static const UINT Offset[RayType::Count] = {
	0,  // Radiance ray
	1  // Shadow ray
};
}
}  // namespace TraceRayParameters
