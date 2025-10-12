#include "ShadersManager.hpp"

#include "engine/core/DxgiInfoManager.hpp"
#include "engine/core/Settings.hpp"

using namespace Microsoft::WRL;

// Helper macro for shader paths
#ifndef SHADER_DIR
#define SHADER_DIR "engine/shaders/"
#endif

// Macro to convert to wide string
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define SHADER_DIR_W WIDEN(SHADER_DIR)

// Helper function to create full shader path
inline std::wstring ShaderPath(const wchar_t* filename)
{
	return std::wstring(SHADER_DIR_W) + filename;
}

void ShadersManager::CompileShaders()
{
	if constexpr (Settings::UseRayTracing())
	{
		CompileRayTracingShaders();
	}
	else
	{
		CompileRasterizationShaders();
	}

	CompileComputeShaders();
}

void ShadersManager::ClearShaders()
{
	shaderMap.clear();
}

void ShadersManager::CompileRasterizationShaders()
{
	///////////////////////////////////////////////////////////////////////
	// Define-uri pentru shadere
	std::vector<std::wstring> lightingDefines = {
		L"MAX_NUMBER_OF_LIGHTS=" + std::to_wstring(Settings::GetMaxNrOfLightSources()),
		L"POINT_LIGHTS_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfPointLights()),
		L"SPOT_LIGHT_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfSpotLights()),
		L"DIR_LIGHTS_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfDirectionalLights())};

	std::vector<std::wstring> waterDefines = {
		L"WAVE_PROPERTIES_COUNT=" + std::to_wstring(Settings::GetNumberOfWaveFunctions())};

	std::vector<std::wstring> includeDirectories = {L"-I", L"Shaders"};
	std::vector<std::wstring> shadowRenderSwitch = {L"SHADOW_RENDER"};

	const auto defineMaxTesselation = [](const float tesselationFactor) -> std::vector<std::wstring>
	{
		std::vector<std::wstring> defines;
		defines.push_back(L"MAX_TESSELATION_FACTOR=" + std::to_wstring(tesselationFactor));
		return defines;
	};


	//////////////////////////////////////////////////////////////////////////////////////////////
	// Compilare shadere

	// Shadere default
	shaderMap["DefaultVS"] = CompileShader(ShaderPath(L"Default.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["DefaultPS"] = CompileShader(ShaderPath(L"Default.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories);

	// Shadere randare shadow map
	shaderMap["ShadowRenderVS"] = CompileShader(ShaderPath(L"Shadow.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["ShadowRenderPS"] = CompileShader(ShaderPath(L"Shadow.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories);

	// Shadere shadow debug
	shaderMap["TextureRenderVS"] =
		CompileShader(ShaderPath(L"TextureRender.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["TextureRenderPS"] =
		CompileShader(ShaderPath(L"TextureRender.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories);

	// Shadere skybox
	shaderMap["SkyBoxVS"] = CompileShader(ShaderPath(L"SkyBox.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["SkyBoxPS"] = CompileShader(ShaderPath(L"SkyBox.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories);

	// Shadere teren
	shaderMap["TerrainVS"] = CompileShader(ShaderPath(L"Terrain.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["TerrainHS"] =
		CompileShader(ShaderPath(L"Terrain.hlsl").c_str(), L"HSMain", L"hs_6_6", includeDirectories, defineMaxTesselation(20.f));
	shaderMap["TerrainDS"] = CompileShader(ShaderPath(L"Terrain.hlsl").c_str(), L"DSMain", L"ds_6_6", includeDirectories);
	shaderMap["TerrainPS"] =
		CompileShader(ShaderPath(L"Terrain.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories, lightingDefines);

	shaderMap["TerrainCubeMapVS"] =
		CompileShader(ShaderPath(L"Terrain.hlsl").c_str(), L"VSForCubeMapRendering", L"vs_6_6", includeDirectories);

	// Shadere apa
	shaderMap["WaterVS"] = CompileShader(ShaderPath(L"Water.hlsl").c_str(), L"VSMain", L"vs_6_6", includeDirectories);
	shaderMap["WaterHS"] =
		CompileShader(ShaderPath(L"Water.hlsl").c_str(), L"HSMain", L"hs_6_6", includeDirectories, defineMaxTesselation(10.f));
	shaderMap["WaterDS"] = CompileShader(ShaderPath(L"Water.hlsl").c_str(), L"DSMain", L"ds_6_6", includeDirectories, waterDefines);
	shaderMap["WaterPS"] =
		CompileShader(ShaderPath(L"Water.hlsl").c_str(), L"PSMain", L"ps_6_6", includeDirectories, lightingDefines);
}

void ShadersManager::CompileRayTracingShaders()
{
	///////////////////////////////////////////////////////////////////////
	// Define-uri pentru shadere
	std::vector<std::wstring> lightingDefines = {
		L"MAX_NUMBER_OF_LIGHTS=" + std::to_wstring(Settings::GetMaxNrOfLightSources()),
		L"POINT_LIGHTS_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfPointLights()),
		L"SPOT_LIGHT_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfSpotLights()),
		L"DIR_LIGHTS_COUNT=" + std::to_wstring(Settings::GetGameSettings().GetMaxNumberOfDirectionalLights())};

	std::vector<std::wstring> includeDirectories = {L"-I", SHADER_DIR_W};


	//////////////////////////////////////////////////////////////////////////////////////////////
	// Compilare shadere
	shaderMap["DefaultRT"] =
		CompileShader(ShaderPath(L"Default.RT.hlsl").c_str(), L"", L"lib_6_6", includeDirectories, lightingDefines);
}

void ShadersManager::CompileComputeShaders()
{
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Compilare shadere
	shaderMap["WavesCS"] = CompileShader(ShaderPath(L"Waves.CS.hlsl").c_str(), L"CSMain", L"cs_6_6");
}

ShadersManager::ShaderPtr ShadersManager::CompileShader(
	const std::wstring& filePath,
	const std::wstring& entryPoint,
	const std::wstring& targetProfile,
	const std::vector<std::wstring>& additionalArguments,
	const std::vector<std::wstring>& defines)
{
	HRESULT hr;

	ComPtr<IDxcCompiler3> pCompiler;
	ComPtr<IDxcLibrary> pLibrary;
	ComPtr<IDxcBlobEncoding> pSourceBlob;
	ComPtr<IDxcIncludeHandler> pIncludeHandler;

	// Incarcam functia DxcCreateInstance direct din fisierul .dll
	typedef HRESULT(WINAPI * DxcCreateInstanceDefinition)(REFCLSID, REFIID, LPVOID*);

	const auto dxCompilerLib = LoadLibraryEx(L"dxcompiler.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (dxCompilerLib == nullptr)
	{
		throw misc::CustomException("Nu a fost gasit fisierul .dll");
	}

	const auto DxcCreateInstance = reinterpret_cast<DxcCreateInstanceDefinition>(
		reinterpret_cast<void*>(GetProcAddress(dxCompilerLib, "DxcCreateInstance")));
	if (DxcCreateInstance == nullptr)
	{
		throw misc::CustomException("Nu a fost gasita functia DxcCreateInstance in .dll");
	}

	// Creem libraria (obiect ajutator) si creem si compilatorul
	GFX_THROW_INFO(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler)));
	GFX_THROW_INFO(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&pLibrary)));

	// Citim in format binar fisierul shader
	GFX_THROW_INFO(pLibrary->CreateBlobFromFile(filePath.c_str(), DXC_CP_ACP, &pSourceBlob));
	GFX_THROW_INFO(pLibrary->CreateIncludeHandler(&pIncludeHandler));

	// Setam argumentele folosite pentru compilare
	std::vector<LPCWSTR> arguments;

	// -E for the entry point (eg. 'main')
	if (entryPoint != L"")
	{
		arguments.push_back(L"-E");
		arguments.push_back(entryPoint.c_str());
	}

	// -T for the target profile (eg. 'ps_6_6')
	arguments.push_back(L"-T");
	arguments.push_back(targetProfile.c_str());

	// Strip reflection data and pdbs (see later)
	arguments.push_back(L"-Qstrip_debug");
	arguments.push_back(L"-Qstrip_reflect");
	arguments.push_back(L"-Qembed_debug");

	// Chestii de debug pentru preluarea erorilor de compilare
	arguments.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);  //-WX
	arguments.push_back(DXC_ARG_DEBUG);  //-Zi


	// Adaugare argumente extra
	for (const std::wstring& additionalArgument : additionalArguments)
	{
		arguments.push_back(additionalArgument.c_str());
	}

	// Adaugare define-uri
	for (const std::wstring& define : defines)
	{
		arguments.push_back(L"-D");
		arguments.push_back(define.c_str());
	}

	DxcBuffer sourceBuffer;
	sourceBuffer.Ptr = pSourceBlob->GetBufferPointer();
	sourceBuffer.Size = pSourceBlob->GetBufferSize();
	sourceBuffer.Encoding = DXC_CP_ACP;

	ComPtr<IDxcResult> pResult;
	GFX_THROW_INFO(pCompiler->Compile(
		&sourceBuffer, arguments.data(), (UINT)arguments.size(), pIncludeHandler.Get(), IID_PPV_ARGS(&pResult)));

	ComPtr<IDxcBlobUtf8> pErrors = nullptr;
	if (SUCCEEDED(pResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr)) && pErrors != nullptr
		&& pErrors->GetStringLength() != 0)
	{
		throw misc::CustomException(pErrors->GetStringPointer());
	}

	if (FAILED(pResult->GetStatus(&hr)) || FAILED(hr))
	{
		throw misc::CustomException("Compilation failed");
	}

	// pResult->GetStatus(&hr);
	// if (FAILED(hr))
	//{
	//	ComPtr<IDxcBlobEncoding> errorsBlob;
	//	pResult->GetErrorBuffer(&errorsBlob);
	//	if (errorsBlob)
	//	{
	//		std::wstring errorMessage = std::wstring(
	//			static_cast<WCHAR*>(errorsBlob->GetBufferPointer()), errorsBlob->GetBufferSize() / sizeof(WCHAR));
	//		throw std::runtime_error(misc::GetStringFromWString(errorMessage));
	//	}
	//	else
	//	{
	//		throw std::runtime_error("Shader compilation failed with unknown error.");
	//	}
	// }

	ComPtr<IDxcBlob> pShaderBlob;
	GFX_THROW_INFO(pResult->GetResult(&pShaderBlob));

	return pShaderBlob;
}

LPVOID ShadersManager::GetBufferPointer(std::string shaderName) const
{
	return shaderMap.at(shaderName).Get()->GetBufferPointer();
}

SIZE_T ShadersManager::GetBufferSize(std::string shaderName) const
{
	return shaderMap.at(shaderName).Get()->GetBufferSize();
}
