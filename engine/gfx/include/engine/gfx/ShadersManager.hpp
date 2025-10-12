#pragma once

#include "Utilities.hpp"

#include <dxcapi.h>
#include <unordered_map>

class ShadersManager
{
public:
	using ShaderPtr = Microsoft::WRL::ComPtr<IDxcBlob>;

public:
	ShadersManager() = default;

	void CompileShaders();
	void ClearShaders();

	LPVOID GetBufferPointer(std::string shaderName) const;
	SIZE_T GetBufferSize(std::string shaderName) const;

private:
	void CompileRasterizationShaders();
	void CompileRayTracingShaders();
	void CompileComputeShaders();

	static ShaderPtr CompileShader(
		const std::wstring& filePath,
		const std::wstring& entryPoint,
		const std::wstring& targetProfile,
		const std::vector<std::wstring>& additionalArguments = {},
		const std::vector<std::wstring>& defines = {});

private:
	mutable std::unordered_map<std::string, ShaderPtr> shaderMap;
};