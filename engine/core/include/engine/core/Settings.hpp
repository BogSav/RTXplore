#pragma once

#include <memory>
#include <Windows.h>

#define INLINE __forceinline

class Settings
{
private:
	Settings() : graphicsSettings(new GraphicsSettings()), gameSettings(new GameSettings()) {}

	class GraphicsSettings
	{
	public:
#pragma warning(push)
#pragma warning(disable : 26495)
		GraphicsSettings() = default;
#pragma warning(pop)

		INLINE int GetWidth() const { return width; }
		INLINE void SetWidth(int toSet)
		{
			width = toSet;
			aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		}
		INLINE int GetHeight() const { return height; }
		INLINE void SetHeight(int toSet)
		{
			height = toSet;
			aspectRatio = static_cast<float>(width) / static_cast<float>(height);
		}
		INLINE void SetWidthAndHeight(int width, int height)
		{
			this->width = width;
			this->height = height;
			aspectRatio = static_cast<float>(this->width) / static_cast<float>(this->height);
		}

		INLINE float GetAspectRatio() { return aspectRatio; }

		INLINE unsigned int GetCbvSrvUavDescriptorSize() const { return cbvSrvUavDescriptorSize; }
		INLINE void SetCbvSrvUavDescriptorSize(unsigned int toSet) { cbvSrvUavDescriptorSize = toSet; }
		INLINE unsigned int GetDsvDescriptorSize() const { return dsvDescriptorSize; }
		INLINE void SetDsvDescriptorSize(unsigned int toSet) { dsvDescriptorSize = toSet; }
		INLINE unsigned int GetRtvDescriptorSize() const { return rtvDescriptorSize; }
		INLINE void SetRtvDescriptorSize(unsigned int toSet) { rtvDescriptorSize = toSet; }

		INLINE bool GetIsMSAAEnabled() { return isMSAASupported && MSAAA_UseFlag; }
		INLINE bool GetIsMSAASupported() { return isMSAASupported; }

		// INLINE void SetIsMSAAEnabled(bool toSet) { MSAAA_UseFlag = toSet; }
		INLINE unsigned int GetMSAAQuality() { return isMSAASupported && MSAAA_UseFlag ? MSAA_Quality : 0; }
		INLINE void SetMSAAQuality(unsigned int toSet) { MSAA_Quality = toSet; }
		INLINE unsigned int GetMSAASampleCount() { return isMSAASupported && MSAAA_UseFlag ? MSAA_SamplesCount : 1; }

		INLINE void SetMSAASampleCount(unsigned int toSet) { MSAA_SamplesCount = toSet; }
		INLINE unsigned int GetMSAAMaximumSampleCount() { return MSAA_SamplesCount; }
		INLINE void SetMSAAMaximumSampleCount(unsigned int toSet) { MSAA_MaximumSampleCount = toSet; }
		INLINE unsigned int GetDesiredMSAASampleCount() { return desiredMSAASampleCount; }
		INLINE void SetIsMSAASupported(bool toSet) { isMSAASupported = toSet; }

		INLINE UINT GetSyncInterval() { return tearingEnabled ? 0 : syncInterval; }
		INLINE void SetSyncValue(UINT toSet) { syncInterval = toSet; }

		INLINE bool IsTearingAllowed() { return tearingEnabled; }
		INLINE void SetTearingEnabled(bool toSet, unsigned int value = 1)
		{
			tearingEnabled = toSet;
			syncInterval = toSet ? 0 : value;
		}

		INLINE void SetUseWarpAdapter(bool toSet) { useWarpAdapter = toSet; }
		INLINE bool GetUseWarpAdapter() { return useWarpAdapter; }

		INLINE bool GetIsZBufferingEnabled() { return isZBufferingEnabled; }
		INLINE bool GetIsAlphaBlendingEnabled() { return isAlphaBlendingEnabled; }

		INLINE bool IsRayTracingSupported() { return isRayTracingSupported; }
		INLINE void SetIsRayTracingSupported(bool toSet) { isRayTracingSupported = toSet; }

		INLINE bool UseBundles() { return useBundles; }

	private:
		friend Settings;

		static constexpr bool useRayTracing = true;
		static constexpr bool useAdvancedReflections = true;
		static constexpr bool useShadows = true;

		static constexpr UINT backBufferCount = 3;
		static constexpr UINT frameResourcesCount = useRayTracing ? 1 : backBufferCount;

		int width = 1200;
		int height = 1200;
		float aspectRatio = 1200.f / 1200.f;

		UINT cbvSrvUavDescriptorSize = 0;
		UINT dsvDescriptorSize = 0;
		UINT rtvDescriptorSize = 0;

		const unsigned int desiredMSAASampleCount = 16;
		const bool MSAAA_UseFlag = false;
		bool isMSAASupported = false;
		UINT MSAA_Quality;
		UINT MSAA_SamplesCount;
		UINT MSAA_MaximumSampleCount;

		bool tearingEnabled = true;
		UINT syncInterval = 1;

		const bool isZBufferingEnabled = true;
		const bool isAlphaBlendingEnabled = false;

		bool useWarpAdapter = false;

		bool isRayTracingSupported = false;

		bool useBundles = true;
	};

	class GameSettings
	{
	public:
#pragma warning(push)
#pragma warning(disable : 26495)
		GameSettings() = default;
#pragma warning(pop)

		INLINE float GetMouseSensitivityOX() { return mouseSensivityOX; }
		INLINE float GetMouseSensitivityOY() { return mouseSensivityOY; }

		INLINE UINT GetMaxNumberOfObjectCB() { return maxNumberOfObjectCB; }
		INLINE UINT GetMaxNumberOfMaterialCB() { return maxNumberOfMaterialCB; }

		INLINE size_t GetMaxNumberOfPointLights() { return maxNumberOfPointLights; }
		INLINE size_t GetMaxNumberOfDirectionalLights() { return maxNumberOfDirectionalLights; }
		INLINE size_t GetMaxNumberOfSpotLights() { return maxNumberOfSpotLights; }

		INLINE size_t GetMaxNumberOfTextures() { return maxNumberOfTextures; }

	private:
		friend Settings;

		float mouseSensivityOX = 0.003f;
		float mouseSensivityOY = 0.003f;

		static constexpr UINT maxNumberOfObjectCB = 20;
		static constexpr UINT maxNumberOfMaterialCB = 10;

		static constexpr size_t maxNumberOfLightSources = 9;
		static constexpr size_t maxNumberOfPointLights = 4;
		static constexpr size_t maxNumberOfSpotLights = 4;
		static constexpr size_t maxNumberOfDirectionalLights = 1;
		static_assert(
			maxNumberOfLightSources == maxNumberOfDirectionalLights + maxNumberOfPointLights + maxNumberOfSpotLights);

		static constexpr size_t numberOfWaveFunctions = 2;

		static constexpr size_t maxNumberOfTextures = 30;
	};

private:
	static Settings mgs;

	std::unique_ptr<GraphicsSettings> graphicsSettings;
	std::unique_ptr<GameSettings> gameSettings;

public:
	static INLINE constexpr unsigned int GetFrameResourcesCount() { return GraphicsSettings::frameResourcesCount; }
	static INLINE constexpr bool UseAdvancedReflections() { return GraphicsSettings::useAdvancedReflections; }
	static INLINE constexpr size_t GetMaxNrOfLightSources() { return GameSettings::maxNumberOfLightSources; }
	static INLINE constexpr size_t GetNumberOfWaveFunctions() { return GameSettings::numberOfWaveFunctions; }
	static INLINE constexpr unsigned int GetBackBufferCount() { return GraphicsSettings::backBufferCount; }
	static INLINE constexpr bool UseRayTracing() { return GraphicsSettings::useRayTracing; }
	static INLINE constexpr bool UseShadows() { return GraphicsSettings::useShadows; }

	static INLINE GraphicsSettings& GetGraphicsSettings() { return *mgs.graphicsSettings; }
	static INLINE GameSettings& GetGameSettings() { return *mgs.gameSettings; }
};