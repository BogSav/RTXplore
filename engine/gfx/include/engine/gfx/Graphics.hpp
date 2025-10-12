#pragma once

#include "CameraController.hpp"
#include "PSO_Manager.hpp"
#include "RS_Manager.hpp"
#include "GraphicsResources.hpp"
#include "LightSource.hpp"
#include "MaterialManager.hpp"
#include "ObjectRenderer.hpp"
#include "SkyBoxRenderer.hpp"
#include "TerrainRenderer.hpp"
#include "TextureManager.hpp"
#include "TextureRenderer.hpp"
#include "WaterRenderer.hpp"

class Graphics
{
public:
	using Ptr = std::unique_ptr<Graphics>;

	Graphics(GraphicsResources& resorurces) : m_graphicsResources(resorurces), m_cameraController(m_camera)
	{
		m_shaderManager.CompileShaders();
		m_RSManager.LoadRootSignatures(GraphicsResources::GetDevice());
		m_computePSOManager.LoadPSOs(m_RSManager, m_shaderManager, GraphicsResources::GetDevice());

		//m_camera = PerspectiveCamera(
		//	Math::Vector3(0, 15, 0),
		//	Math::Vector3(-1, 15, 0),
		//	Math::Vector3{0.f, 1.f, 0.f},
		//	Settings::GetGraphicsSettings().GetAspectRatios()
		//	/*, DirectX::XMConvertToRadians(90)*/);
		m_camera = PerspectiveCamera(
			Math::Vector3(-40.247837, 7.752207, -21.161358),
			Math::Vector3(-40.247837, 7.752207, -21.161358) + Math::Vector3(0.599414, -0.586723, -0.544481),
			Math::Vector3(0.434299, 0.809788, -0.394497),
			Settings::GetGraphicsSettings().GetAspectRatio()
			/*, DirectX::XMConvertToRadians(90)*/);
		m_cameraController.InstantiateFrustum();
	};

	virtual void Update(float deltaTime, float totalTime) = 0;
	void SetRenderMode(UINT toSet) { m_renderMode = toSet; }

	void Render()
	{
		m_graphicsResources.BeginFrame();

		this->PopulateCommandList();

		m_graphicsResources.EndFrame();
	}

	const PerspectiveCamera& GetCamera() const { return m_camera; }
	CameraController& GetCameraController() { return m_cameraController; }

	virtual ~Graphics() { OutputDebugString(L"Graphics object destroyed\n\n\n"); }

private:
	virtual void LoadAssets() = 0;
	virtual void PopulateCommandList() = 0;

protected:
	GraphicsResources& m_graphicsResources;

	PSO_Manager<ComputePSO> m_computePSOManager;

	ObjectRenderer::Ptr m_objectRenderer;
	TerrainRenderer::Ptr m_terrainRender;
	WaterRenderer::Ptr m_waterRenderer;
	SkyBoxRenderer::Ptr m_skyBoxRenderer;

	ShadersManager m_shaderManager;
	RS_Manager m_RSManager;

	LightSource::Vec m_lightSources;

	MaterialManager m_materialManager;
	TextureManager m_textureManager;

	CameraController m_cameraController;
	PerspectiveCamera m_camera;

	UINT m_renderMode = RenderMode::Everything;
};