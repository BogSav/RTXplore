#include "DX_FrameResources.hpp"

#include "Camera.hpp"
#include "CameraController.hpp"
#include "DX_DynamicCubeMap.hpp"
#include "DX_PSO.hpp"
#include "DX_ShadowMap.hpp"
#include "engine/core/DxgiInfoManager.hpp"
#include "GraphicsResources.hpp"
#include "LightSource.hpp"
#include "MaterialManager.hpp"
#include "ObjectRenderer.hpp"
#include "SkyBoxRenderer.hpp"
#include "TerrainRenderer.hpp"
#include "TextureManager.hpp"
#include "WaterRenderer.hpp"

using namespace DirectX;

UINT FrameResources::objectCB_ID = 0;
UINT FrameResources::materialCB_ID = 0;

void FrameResources::Create()
{
	m_perMaterialCB.Create(
		GraphicsResources::GetDevice(), Settings::GetGameSettings().GetMaxNumberOfMaterialCB(), L"PerMaterialCB");
	m_perObjectCB.Create(
		GraphicsResources::GetDevice(), Settings::GetGameSettings().GetMaxNumberOfObjectCB(), L"PerObjectCB");
	m_waterCB.Create(GraphicsResources::GetDevice(), 1, L"WaterCB");

	if (!Settings::UseRayTracing())
	{
		m_perPassCB.Create(
			GraphicsResources::GetDevice(),
			8 /*1 + (Settings::UseAdvancedReflections() ? 6 : 0) + (Settings::UseShadows() ? 1 : 0)*/,
			L"PerPassCB");
	}
	else
	{
		m_perPassCB.Create(GraphicsResources::GetDevice(), 1, L"PerPassCB");
	}
}

void FrameResources::UpdateObjectRendererCB(const ObjectRenderer& objectRenderer)
{
	for (const auto& object : objectRenderer.GetObjects())
	{
		this->UpdatePerObjectCB(*object);
	}
}

void FrameResources::UpdatePerObjectCB(const Object& object)
{
	// Matrice transformare
	XMStoreFloat4x4(&m_perObjectCB->worldMatrix, XMMatrixTranspose(object.GetTransform()));

	// Matricea de transformare inversa
	XMVECTOR determinant = XMMatrixDeterminant(object.GetTransform());
	XMStoreFloat4x4(
		&m_perObjectCB->invWorldMatrix, XMMatrixTranspose(XMMatrixInverse(&determinant, object.GetTransform())));

	// Transformarea de textura
	XMStoreFloat4x4(&m_perObjectCB->textureTransform, XMMatrixTranspose(object.GetTextureTransform()));

	m_perObjectCB.CopyStagingToGpu(object.GetObjectCB_ID());
}

void FrameResources::UpdateSkyBoxCB(const SkyBoxRenderer& skyBox)
{
	this->UpdatePerObjectCB(skyBox);
}

void FrameResources::UpdateWaterCB(const WaterRenderer& water)
{
	m_waterCB->cubeMapCenter = water.GetCubeMapCenter();
	m_waterCB->cubeMapSphereRadius = water.GetCubeMapSphereRadius();
	m_waterCB->waterColor = water.GetColor();

	for (int i = 0; i < Settings::GetNumberOfWaveFunctions(); i++)
	{
		m_waterCB->waveParameters[i] = water.GetWaveParameters(i);
	}

	m_waterCB.CopyStagingToGpu();

	this->UpdatePerObjectCB(water);
}

void FrameResources::UpdatePerMaterialCB(const MaterialManager& materialManager)
{
	for (const auto& material : materialManager.GetMaterials())
	{
		m_perMaterialCB.CopyData(material.GetMaterialCB_ID(), material.GetMaterialProperties());
	}
}

void FrameResources::UpdateTerrainCB(const TerrainRenderer& terrain)
{
	this->UpdatePerObjectCB(terrain);
}

void FrameResources::UpdateMainPassCB(
	const BaseCamera& camera,
	const float& deltaTime,
	const float& totalTime,
	const UINT& renderMode,
	LightSource::Vec& lightSources)
{
	// Doar provizorii, in realitate dimensiunea poate varia
	m_perPassCB->renderTargetSize =
		XMFLOAT2((float)Settings::GetGraphicsSettings().GetWidth(), (float)Settings::GetGraphicsSettings().GetHeight());
	m_perPassCB->invRenderTargetSize =
		XMFLOAT2(1.f / Settings::GetGraphicsSettings().GetWidth(), 1.f / Settings::GetGraphicsSettings().GetHeight());

	// DeltaTime si totalTime
	m_perPassCB->deltaTime = deltaTime;
	m_perPassCB->totalTime = totalTime;

	m_perPassCB->renderMode = renderMode;
	m_perPassCB->ambientLight = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.f);
	m_perPassCB->fogColor = XMFLOAT3(130.f / 255.f, 133.f / 255.f, 149.f / 255.f);
	m_perPassCB->fogStart = 100.0f;

	for (const auto& lightSource : lightSources)
	{
		m_perPassCB->lightSources[lightSource.GetID()] = lightSource.m_lightProperties;
	}

	UpdateCameraAndTransferToGPUPassCB(camera, 0);
}

void FrameResources::UpdateShadowMapPassCB(const ShadowMap& shadowMap)
{
	UpdateCameraAndTransferToGPUPassCB(shadowMap.GetCamera(), 7);
}

void FrameResources::UpdateDyanmicCubeMapPassCB(const DynamicCubeMap& dynamicCubeMap)
{
	for (int i = 0; i < 6; i++)
	{
		UpdateCameraAndTransferToGPUPassCB(dynamicCubeMap.GetCameraController().GetCamera(i), i + 1);
	}
}

void FrameResources::UpdateCameraAndTransferToGPUPassCB(const BaseCamera& camera, int i)
{
	// View si invView
	XMStoreFloat4x4(&m_perPassCB->viewMatrix, XMMatrixTranspose(camera.GetViewMatrix()));
	XMStoreFloat4x4(&m_perPassCB->invViewMatrix, XMMatrixTranspose(camera.GetInverseViewMatrix()));

	// Proj si invProj
	XMStoreFloat4x4(&m_perPassCB->projMatrix, XMMatrixTranspose(camera.GetProjectionMatrix()));
	XMStoreFloat4x4(
		&m_perPassCB->invProjMatrix, XMMatrixTranspose(Math::Matrix4::Inverse(camera.GetProjectionMatrix())));

	// ViewProj si invViewProj
	Math::Matrix4 viewProj = camera.GetViewMatrix() * camera.GetProjectionMatrix();

	XMStoreFloat4x4(&m_perPassCB->viewProjMatrix, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&m_perPassCB->invViewProjMatrix, XMMatrixTranspose(Math::Matrix4::Inverse(viewProj)));

	// EyePosition
	XMStoreFloat3(&m_perPassCB->eyePosition, camera.GetPosition());

	// Far/near Z
	m_perPassCB->farZ = camera.GetZFar();
	m_perPassCB->nearZ = camera.GetZNear();

	m_perPassCB.CopyStagingToGpu(i);
}
