#include "pch.h"
#include "Renderer.h"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define Purple "\033[35m"
#define RESET "\033[0m"

namespace dae 
{
	Renderer::Renderer(SDL_Window* pWindow) 
		: m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//DirectX-------------------------

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			//std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		//Software------------------------

		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];
		ResetDepthBuffer();
		
		//--------------------------------

		m_Camera.Initialize(45.f, Vector3{ 0.f, 0.f, -50.f }, static_cast<float>(m_Width) / static_cast<float>(m_Height));

		InitializeDirectXMeshes();
		InitializeSoftwareMeshes();

		PrintInfo();
	}

	Renderer::~Renderer()
	{
		DeleteDirectXResources();
		DeleteSoftwareResources();
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		UpdateDirectX(pTimer);
		UpdateSoftware(pTimer);
	}

	void Renderer::Render()
	{
		switch (m_RenderStyle)
		{
		case dae::Renderer::RenderingStyle::Software:
			RenderSoftware();
			break;
		case dae::Renderer::RenderingStyle::DirectX:
			RenderDirectX();
			break;
		default:
			break;
		}
	}

	void Renderer::EnableRotation()
	{
		std::cout << RED;

		m_Rotating = !m_Rotating;
		std::cout << "Rotation ";
		if (m_Rotating)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET;
	}
	void Renderer::CycleRenderStyle()
	{
		m_RenderStyle = static_cast<RenderingStyle>((static_cast<int>(m_RenderStyle) + 1) % (static_cast<int>(RenderingStyle::DirectX) + 1));

		std::cout << RED;

		std::cout << "RenderStyle set to: ";
		switch (m_RenderStyle)
		{
		case dae::Renderer::RenderingStyle::Software:
			std::cout << "software\n";
			break;
		case dae::Renderer::RenderingStyle::DirectX:
			std::cout << "DirectX\n";
			break;
		default:
			break;
		}

		std::cout << RESET;
	}
	void Renderer::CycleCullModes()
	{
		std::cout << RED;

		for (auto& mesh : m_vecMeshes)
		{
			mesh->CycleCullMode();
		}

		std::cout << RESET;
	}

	void Renderer::TogglePrintFPS()
	{
		m_PrintFPS = !m_PrintFPS;
		std::cout << RED << "Print fps ";
		if (m_PrintFPS)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET; //Reset
	}
	void Renderer::ToggleUniformClearColor()
	{
		std::cout << RED;

		m_IsUniformColorEnabled = !m_IsUniformColorEnabled;
		std::cout << "Uniform color ";
		if (m_IsUniformColorEnabled)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}


		std::cout << RESET;
	}
	void Renderer::PrintInfo() const
	{
		std::cout << RED; // set console Red
		std::cout << "[Key Bindings - SHARED]" << '\n';
		std::cout << '\t' << "[F1]"		<< '\t' << "Toggle Rasterizer Mode"				<< '\t' << '\t' << "(HARDWARE/SOFTWARE)"						<< '\n';
		std::cout << '\t' << "[F2]"		<< '\t' << "Toggle Vehicle Rotation"			<< '\t' << '\t' << "(ON/OFF)"									<< '\n';
		std::cout << '\t' << "[F9]"		<< '\t' << "Cycle CullMode"						<< '\t' << '\t' << '\t' << "(BACK/FRONT/NONE)"					<< '\n';
		std::cout << '\t' << "[F10]"	<< '\t' << "Toggle Uniform ClearColor"			<< '\t'			<< "(ON/OFF)"									<< '\n';
		std::cout << '\t' << "[F11]"	<< '\t' << "Toggle Print FPS"					<< '\t' << '\t' << "(OF/OFF)"									<< '\n';
		std::cout << '\n';

		std::cout << GREEN; // set console Green
		std::cout << "[Key Bindings - HARDWARE]" << '\n';
		std::cout << '\t' << "[F3]"		<< '\t' << "Toggle FireFX"						<< '\t' << '\t' << '\t' << "(ON/OFF)"							<< '\n';
		std::cout << '\t' << "[F4]"		<< '\t' << "Cycle Sampler State"				<< '\t' << '\t' << "(POINT/LINEAR/ANISOTROPIC)"					<< '\n';
		std::cout << '\n';

		std::cout << Purple; // set console Purple
		std::cout << "[Key Bindings - SOFTWARE]" << '\n';
		std::cout << '\t' << "[F5]"		<< '\t' << "Cycle Shading Mode"					<< '\t' << '\t' << "(COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)"	<< '\n';
		std::cout << '\t' << "[F6]"		<< '\t' << "Toggle NormalMap"					<< '\t' << '\t' << "(ON/OFF)"									<< '\n';
		std::cout << '\t' << "[F7]"		<< '\t' << "Toggle DepthBuffer Visual"			<< '\t'			<< "(BACK/FRONT/NONE)"							<< '\n';
		std::cout << '\t' << "[F8]"		<< '\t' << "Toggle BoundingBox Visual"			<< '\t'			<< "(ON/OFF)"									<< '\n';
		std::cout << '\n';

		std::cout << RESET; //Reset
	}

	//Software ------------------------------------------------------------------
	void Renderer::InitializeSoftwareMeshes()
	{
		m_pNormalTexture	 = Software_Texture::LoadFromFile("Resources/vehicle_normal.png");
		m_pDiffuseTexture	 = Software_Texture::LoadFromFile("Resources/vehicle_diffuse.png");
		m_pSpecularTexture	 = Software_Texture::LoadFromFile("Resources/vehicle_specular.png");
		m_pGlossinessTexture = Software_Texture::LoadFromFile("Resources/vehicle_gloss.png");


		Utils::ParseOBJ("Resources/vehicle.obj", m_Mesh.vertices, m_Mesh.indices);
		const Vector3 position{ m_Camera.origin + Vector3{ 0.0f, 0.0f, 50.0f } };
		const Vector3 scale{ Vector3{ 1.0f, 1.0f, 1.0f } };

		const Vector3 rotation{ };
		m_Mesh.worldMatrix = Matrix::CreateScale(scale) * Matrix::CreateRotation(rotation) * Matrix::CreateTranslation(position);
	}
	void Renderer::DeleteSoftwareResources()
	{
		delete[] m_pDepthBufferPixels;

		delete m_pTexture;
		m_pTexture = nullptr;
		
		delete m_pNormalTexture;
		m_pNormalTexture = nullptr;
		
		delete m_pDiffuseTexture;
		m_pDiffuseTexture = nullptr;
		
		delete m_pSpecularTexture;
		m_pSpecularTexture = nullptr;
		
		delete m_pGlossinessTexture;
		m_pGlossinessTexture = nullptr;
	}
	void Renderer::UpdateSoftware(const Timer* pTimer)
	{
		if (m_Rotating)
		{
			const float rotationSpeed{ 30.f };
			m_Mesh.RotateY(rotationSpeed * pTimer->GetElapsed());
		}
	}
	void Renderer::RenderSoftware()
	{
		//@START
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);
		VertexTransformationFunction(m_Mesh);

		std::vector<Vector2> verticesScreenSpace;
		for (const auto& ndcVertex : m_Mesh.vertices_out)
		{
			Vector2 vertex;
			vertex.x = ((ndcVertex.position.x + 1) / 2) * m_Width;
			vertex.y = ((1 - ndcVertex.position.y) / 2) * m_Height;

			verticesScreenSpace.push_back(vertex);
		}

		ResetDepthBuffer();
		ClearBackground();

		switch (m_Mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleList:
			for (int index{ 0 }; index < m_Mesh.indices.size(); index += TRIANGLE_SIDES)
			{
				RenderTriangle(m_Mesh, verticesScreenSpace, index, false);
			}
			break;
		case PrimitiveTopology::TriangleStrip:
			for (int index{ 0 }; index < m_Mesh.indices.size() - 2; ++index)
			{
				RenderTriangle(m_Mesh, verticesScreenSpace, index, index % 2);
			}
			break;
		default:
			//if this is selected, no topoly is selected -- should not happen
			break;
		}


		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void Renderer::VertexTransformationFunction(Mesh& mesh)
	{
		Matrix worldViewProjectionMatrix{ mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

		mesh.vertices_out.clear();
		mesh.vertices_out.reserve(mesh.vertices.size());

		for (const Vertex& v : mesh.vertices)
		{
			Vertex_Out vertex_out{ Vector4{}, v.color, v.uv, v.normal, v.tangent };

			vertex_out.position = worldViewProjectionMatrix.TransformPoint({ v.position, 1.0f });
			vertex_out.viewDirection = Vector3{ vertex_out.position.x, vertex_out.position.y, vertex_out.position.z }.Normalized();

			vertex_out.normal = mesh.worldMatrix.TransformVector(v.normal);
			vertex_out.tangent = mesh.worldMatrix.TransformVector(v.tangent);

			const float invVw{ 1 / vertex_out.position.w };
			vertex_out.position.x *= invVw;
			vertex_out.position.y *= invVw;
			vertex_out.position.z *= invVw;

			mesh.vertices_out.emplace_back(vertex_out);
		}
	}
	void Renderer::RenderTriangle(const Mesh& mesh, const std::vector<Vector2>& screenSpaceVertices, int vertexIndex, bool swapVertices)
	{
		const size_t vertexIndex0{ mesh.indices[vertexIndex + (2 * swapVertices)] };
		const size_t vertexIndex1{ mesh.indices[vertexIndex + 1] };
		const size_t vertexIndex2{ mesh.indices[vertexIndex + (!swapVertices * 2)] };

		if (vertexIndex0 == vertexIndex1 || vertexIndex1 == vertexIndex2 || vertexIndex2 == vertexIndex0)
		{
			return;
		}

		const Vector2 vertex0{ screenSpaceVertices[vertexIndex0] };
		const Vector2 vertex1{ screenSpaceVertices[vertexIndex1] };
		const Vector2 vertex2{ screenSpaceVertices[vertexIndex2] };

		const Vector2 edge0{ vertex1 - vertex0 };
		const Vector2 edge1{ vertex2 - vertex1 };
		const Vector2 edge2{ vertex0 - vertex2 };


		const float area{ Vector2::Cross(edge0, edge1) };
		const float invArea{ 1 / area };

		//bounding box
		Vector2 topLeft{ Vector2::Min(vertex0, Vector2::Min(vertex1, vertex2)) };
		Vector2 bottomRight{ Vector2::Max(vertex0, Vector2::Max(vertex1, vertex2)) };

		const float margin{ 1 };

		topLeft.x = Clamp(topLeft.x - margin, 0.f, static_cast<float>(m_Width));
		topLeft.y = Clamp(topLeft.y - margin, 0.f, static_cast<float>(m_Height));
		bottomRight.x = Clamp(bottomRight.x + margin, 0.f, static_cast<float>(m_Width));
		bottomRight.y = Clamp(bottomRight.y + margin, 0.f, static_cast<float>(m_Height));

		const int startX{ static_cast<int>(topLeft.x) };
		const int endX{ static_cast<int>(bottomRight.x) };

		const int startY{ static_cast<int>(topLeft.y) };
		const int endY{ static_cast<int>(bottomRight.y) };

		for (int px{ startX }; px < endX; ++px)
		{
			for (int py{ startY }; py < endY; ++py)
			{
				Vector2 currentPixel{ static_cast<float>(px), static_cast<float>(py) };
				const int pixelIndex{ px + py * m_Width };

				if (m_ShowBoundingBox)
				{
					m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255),
						static_cast<uint8_t>(255));

					continue;
				}

				bool hitTriangle{ Utils::IsInTriangle(currentPixel, vertex0, vertex1, vertex2) };
				if (hitTriangle)
				{
					//Calculate weights
					float weight0{ Vector2::Cross((currentPixel - vertex1), vertex1 - vertex2) };
					float weight1{ Vector2::Cross((currentPixel - vertex2), vertex2 - vertex0) };
					float weight2{ Vector2::Cross((currentPixel - vertex0), vertex0 - vertex1) };

					weight0 *= invArea;
					weight1 *= invArea;
					weight2 *= invArea;

					const float depth0{ mesh.vertices_out[vertexIndex0].position.z };
					const float depth1{ mesh.vertices_out[vertexIndex1].position.z };
					const float depth2{ mesh.vertices_out[vertexIndex2].position.z };

					const float interpolatedZDepth{ 1.f / (weight0 * (1.f / depth0) + weight1 * (1.f / depth1) + weight2 * (1.f / depth2)) };

					if (m_pDepthBufferPixels[pixelIndex] < interpolatedZDepth)
					{
						continue;
					}

					m_pDepthBufferPixels[pixelIndex] = interpolatedZDepth;


					Vertex_Out pixel{};

					const float interpolatedWDepth{ 1.f / (weight0 * (1.f / mesh.vertices_out[vertexIndex0].position.w) + weight1 * (1.f / mesh.vertices_out[vertexIndex1].position.w) + weight2 * (1.f / mesh.vertices_out[vertexIndex2].position.w)) };

					const Vector2 UV
					{
						(
							weight0 * mesh.vertices_out[vertexIndex0].uv / mesh.vertices_out[vertexIndex0].position.w +
							weight1 * mesh.vertices_out[vertexIndex1].uv / mesh.vertices_out[vertexIndex1].position.w +
							weight2 * mesh.vertices_out[vertexIndex2].uv / mesh.vertices_out[vertexIndex2].position.w
						) * interpolatedWDepth
					};


					const Vector3 normal
					{
						(
							weight0 * mesh.vertices_out[vertexIndex0].normal / mesh.vertices_out[vertexIndex0].position.w +
							weight1 * mesh.vertices_out[vertexIndex1].normal / mesh.vertices_out[vertexIndex1].position.w +
							weight2 * mesh.vertices_out[vertexIndex2].normal / mesh.vertices_out[vertexIndex2].position.w
						) * interpolatedWDepth
					};

					const Vector3 tangent
					{
						(
							weight0 * mesh.vertices_out[vertexIndex0].tangent / mesh.vertices_out[vertexIndex0].position.w +
							weight1 * mesh.vertices_out[vertexIndex1].tangent / mesh.vertices_out[vertexIndex1].position.w +
							weight2 * mesh.vertices_out[vertexIndex2].tangent / mesh.vertices_out[vertexIndex2].position.w
						) * interpolatedWDepth
					};

					const Vector3 viewDirection
					{
						(
							weight0 * mesh.vertices_out[vertexIndex0].viewDirection / mesh.vertices_out[vertexIndex0].position.w +
							weight1 * mesh.vertices_out[vertexIndex1].viewDirection / mesh.vertices_out[vertexIndex1].position.w +
							weight2 * mesh.vertices_out[vertexIndex2].viewDirection / mesh.vertices_out[vertexIndex2].position.w
						) * interpolatedWDepth
					};

					pixel.uv = UV;
					pixel.normal = normal.Normalized();
					pixel.tangent = tangent.Normalized();
					pixel.viewDirection = viewDirection.Normalized();


					if (m_ShowDepthBuffer)
					{
						const float depthColor{ Remap(interpolatedZDepth, 0.985f, 1.0f) };
						pixel.color = { depthColor, depthColor, depthColor };
					}

					PixelShading(px + (py * m_Width), pixel);
				}
			}
		}
	}

	void Renderer::PixelShading(int pixelIndex, const Vertex_Out& pixel) const
	{
		Vector3 pixelNormal{ pixel.normal };

		if (m_IsNormalMapEnabled)
		{
			const Vector3 binormal{ Vector3::Cross(pixel.normal, pixel.tangent) };
			const Matrix tangentSpaceAxis{ Matrix{pixel.tangent, binormal, pixel.normal, Vector3::Zero} };

			const ColorRGB normalMap{ (2 * m_pNormalTexture->Sample(pixel.uv)) - ColorRGB{1,1,1} };
			const Vector3 normalSample{ normalMap.r, normalMap.g, normalMap.b };
			pixelNormal = tangentSpaceAxis.TransformVector(normalSample);
		}

		Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
		lightDirection.Normalize();

		const float lightIntensity{ 7.f };
		const float specularShinyValue{ 25.f };


		ColorRGB finalColor{};

		const float observedArea{ Vector3::DotClamped(pixelNormal.Normalized(), -lightDirection) };

		switch (m_LightingMode)
		{
		case dae::Renderer::LightingMode::Combined:
		{
			const ColorRGB lambert{ LightingUtils::Lambert(1.0f, m_pDiffuseTexture->Sample(pixel.uv)) };
			const float specularExp{ specularShinyValue * m_pGlossinessTexture->Sample(pixel.uv).r };
			const ColorRGB specular{ m_pSpecularTexture->Sample(pixel.uv) * LightingUtils::Phong(1.0f, specularExp, -lightDirection, pixel.viewDirection, pixelNormal) };
			finalColor += lightIntensity * observedArea * lambert + specular;
		}
		break;
		case dae::Renderer::LightingMode::ObservedArea:
		{
			finalColor += ColorRGB{ observedArea, observedArea, observedArea };
		}
		break;
		case dae::Renderer::LightingMode::Diffuse:
		{
			const ColorRGB lambert{ LightingUtils::Lambert(1.0f, m_pDiffuseTexture->Sample(pixel.uv)) };
			finalColor += lightIntensity * observedArea * lambert;
		}
		break;
		case dae::Renderer::LightingMode::Specular:
		{
			const float specularExp{ specularShinyValue * m_pGlossinessTexture->Sample(pixel.uv).r };
			const ColorRGB specular{ m_pSpecularTexture->Sample(pixel.uv) * LightingUtils::Phong(1.0f, specularExp, -lightDirection, pixel.viewDirection, pixelNormal) };
			finalColor += observedArea * specular;
		}
		break;
		default:
			break;
		}

		if (m_ShowDepthBuffer)
		{
			finalColor += pixel.color;
		}

		//Update Color in Buffer
		finalColor.MaxToOne();

		m_pBackBufferPixels[pixelIndex] = SDL_MapRGB(m_pBackBuffer->format,
			static_cast<uint8_t>(finalColor.r * 255),
			static_cast<uint8_t>(finalColor.g * 255),
			static_cast<uint8_t>(finalColor.b * 255));

	}

	void Renderer::ClearBackground() const
	{
		if (m_IsUniformColorEnabled)
		{
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_UniformColor.r * 100), static_cast<Uint8>(m_UniformColor.g * 100), static_cast<Uint8>(m_UniformColor.b * 100)));
		}
		else
		{
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(m_SoftwareColor.r * 100), static_cast<Uint8>(m_SoftwareColor.g * 100), static_cast<Uint8>(m_SoftwareColor.b * 100)));
		}

	}
	void Renderer::ResetDepthBuffer()
	{
		std::fill_n(m_pDepthBufferPixels, (m_Width * m_Height), FLT_MAX);
	}

	void Renderer::CycleShadingMode()
	{
		std::cout << GREEN;

		m_LightingMode = static_cast<LightingMode>((static_cast<int>(m_LightingMode) + 1) % (static_cast<int>(LightingMode::Specular) + 1));
		std::cout << "Shading set to: ";
		switch (m_LightingMode)
		{
		case dae::Renderer::LightingMode::Combined:
			std::cout << "combined\n";
			break;
		case dae::Renderer::LightingMode::ObservedArea:
			std::cout << "ObservedArea\n";
			break;
		case dae::Renderer::LightingMode::Diffuse:
			std::cout << "Diffuse\n";
			break;
		case dae::Renderer::LightingMode::Specular:
			std::cout << "Specular\n";
			break;
		default:
			break;
		}

		std::cout << RESET;
	}
	void Renderer::ToggleNormalMap()
	{
		std::cout << GREEN;

		m_IsNormalMapEnabled = !m_IsNormalMapEnabled;
		std::cout << "NormalMap ";
		if (m_IsNormalMapEnabled)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET;
	}
	void Renderer::ToggleDepthBuffer()
	{
		std::cout << GREEN;

		m_ShowDepthBuffer = !m_ShowDepthBuffer;
		std::cout << "DepthBuffer ";
		if (m_ShowDepthBuffer)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET;
	}
	void Renderer::ToggleBoundingBoxVisualization()
	{
		std::cout << GREEN;

		m_ShowBoundingBox = !m_ShowBoundingBox;
		std::cout << "BoundingBox ";
		if (m_ShowBoundingBox)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET;
	}

	//DirectX --------------------------------------------------------------------
	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		//========

		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel, 1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);

		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));

		if (FAILED(result))
			return result;


		//2. Create Swapchain
		//======
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get handle from the SDL backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//view
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilViewDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		//Create actial resource and matching resource view
		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//======

		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//view
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5. Bind RTV & DSV to Output Merger Stage
		//=======
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set viewport
		//=========

		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return S_OK;
	}
	void Renderer::InitializeDirectXMeshes()
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		//Vehicle
		bool parse{ Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices) };
		if (parse == false)
		{
			std::cout << "parse failed\n";
		}

		Effect_Shaded* pShadedEffect = new Effect_Shaded(m_pDevice, L"Resources/PosTex3D.fx");

		DirectX_Texture diffuseTexture{ "Resources/vehicle_diffuse.png",	m_pDevice };
		DirectX_Texture normalTexture{ "Resources/vehicle_normal.png",		m_pDevice };
		DirectX_Texture specularTexture{ "Resources/vehicle_specular.png",	m_pDevice };
		DirectX_Texture glossinessTexture{ "Resources/vehicle_gloss.png",		m_pDevice };

		pShadedEffect->SetDiffuseMap(&diffuseTexture);
		pShadedEffect->SetNormalMap(&normalTexture);
		pShadedEffect->SetSpeculareMap(&specularTexture);
		pShadedEffect->SetGlossinessMap(&glossinessTexture);

		m_vecMeshes.push_back(new mesh(m_pDevice, vertices, indices, pShadedEffect));

		//Fire
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);
		effect* pEffect = new effect(m_pDevice, L"Resources/Transparency.fx");

		DirectX_Texture fireDiffuseTexture{ "Resources/fireFX_diffuse.png",	m_pDevice };
		pEffect->SetDiffuseMap(&fireDiffuseTexture);

		m_vecMeshes.push_back(new mesh(m_pDevice, vertices, indices, pEffect));
	}
	void Renderer::DeleteDirectXResources()
	{
		for (auto& mesh : m_vecMeshes)
		{
			delete mesh;
		}

		//RELEASE RESOURCES IN REVERSED ORDER
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}
	}
	void Renderer::UpdateDirectX(const Timer* pTimer)
	{
		const float rotationSpeed{ 30.f };

		for (auto& mesh : m_vecMeshes)
		{
			if (m_Rotating)
			{
				mesh->RotateY(rotationSpeed * TO_RADIANS * pTimer->GetElapsed());
			}

			mesh->UpdateMatrices(m_Camera.GetWorldViewProjection(), m_Camera.GetInvViewMatrix());
		}
	}
	void Renderer::RenderDirectX() const
	{
		if (!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		ColorRGB clearColor = ColorRGB{ 0.f, 0.f, 0.3f };
		if (m_IsUniformColorEnabled)
		{
			clearColor = m_UniformColor;
		}		
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);


		//2. SET PIPELINE + INVOKE DRAWCALLS ( = RENDER)
		m_vecMeshes[0]->Render(m_pDeviceContext); //Vehicle
		if (m_ShowFire)
		{
			m_vecMeshes[1]->Render(m_pDeviceContext); //Vehicle
		}

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::CycleFilteringMethods()
	{
		std::cout << Purple;

		for (auto& mesh : m_vecMeshes)
		{
			mesh->CycleFilteringMethod();
		}

		std::cout << RESET;
	}
	void Renderer::ToggleFireFx()
	{
		std::cout << Purple;

		m_ShowFire = !m_ShowFire;

		std::cout << "Fire ";
		if (m_ShowFire)
		{
			std::cout << "Enabled\n";
		}
		else
		{
			std::cout << "Dissabled\n";
		}

		std::cout << RESET;
	}
}