#pragma once
#include "DataTypes.h"

#include "Utils.h"
#include "mesh.h"
#include "Camera.h"


using namespace dae;

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render();

		void CycleRenderStyle();				//F1
		void EnableRotation();					//F2
		void ToggleFireFx();					//F3
		void CycleFilteringMethods();			//F4
		void CycleShadingMode();				//F5
		void ToggleNormalMap();					//F6
		void ToggleDepthBuffer();				//F7
		void ToggleBoundingBoxVisualization();	//F8

		void CycleCullModes();					//F9
		void ToggleUniformClearColor();			//F10
		void TogglePrintFPS();					//F11

		bool PrintFps() const
		{
			return m_PrintFPS;
		}

	private:
		enum class RenderingStyle
		{
			Software,
			DirectX
		};
		RenderingStyle m_RenderStyle{ RenderingStyle::DirectX };

		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		Camera m_Camera{};
		bool m_Rotating{ true };
		bool m_PrintFPS{ false };
		bool m_IsUniformColorEnabled{ false };

		void PrintInfo() const;

		ColorRGB m_UniformColor{ 0.1f, 0.1f, 0.1f };
		ColorRGB m_SoftwareColor{ 0.39f, 0.59f, 0.99f };
		ColorRGB m_HardwareColor{ 0.39f, 0.39f ,0.39f };


		//Software Variables -----------------------------
		bool m_ShowDepthBuffer{ false };
		bool m_ShowBoundingBox{ false };
		bool m_IsNormalMapEnabled{ true };

		enum class LightingMode
		{
			Combined,
			ObservedArea,
			Diffuse,
			Specular
		};
		LightingMode m_LightingMode{ LightingMode::Combined };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};
		const int TRIANGLE_SIDES{ 3 };

		Mesh m_Mesh{};

		//TODO Fix texture
		Software_Texture* m_pTexture{};
		Software_Texture* m_pNormalTexture{};
		Software_Texture* m_pDiffuseTexture{};
		Software_Texture* m_pSpecularTexture{};
		Software_Texture* m_pGlossinessTexture{};

		//Software Functions -----------------------------
		void VertexTransformationFunction(Mesh& mesh);
		void RenderTriangle(const Mesh& mesh, const std::vector<Vector2>& screenSpaceVertices, int vertexIndex, bool swapVertices);
		void PixelShading(int pixelIndex, const Vertex_Out& pixel) const;
		
		void ClearBackground() const;
		void ResetDepthBuffer();

		void InitializeSoftwareMeshes();
		void DeleteSoftwareResources();
		void UpdateSoftware(const Timer* pTimer);
		void RenderSoftware();


		//DirectX Variables ------------------------------
		bool m_ShowFire{ true };

		bool m_IsInitialized{ false };		
		std::vector<mesh*> m_vecMeshes;
		
		//DIRECTX
		HRESULT InitializeDirectX();
		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};
		IDXGISwapChain* m_pSwapChain{};
		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};
		ID3D11Resource* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};	

		//DirectX Function -------------------------------
		void InitializeDirectXMeshes();
		void DeleteDirectXResources();
		void UpdateDirectX(const Timer* pTimer);
		void RenderDirectX() const;

	};
}
