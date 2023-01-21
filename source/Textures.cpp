#include "pch.h"
#include "Textures.h"
#include "Vector2.h"

DirectX_Texture::DirectX_Texture(const std::string& path, ID3D11Device* pDevice)
{
	// Make SDL_Surface, release at the end
	SDL_Surface* pSurface = IMG_Load(path.c_str());

	// Texture description
	const DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width	= pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	// InitData SDL_Surface
	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);

	// ShaderResourceView description
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = 1;

	hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);

	// SDL_Surface no longer needed
	SDL_FreeSurface(pSurface);
}

DirectX_Texture::~DirectX_Texture()
{
	if (m_pResource)
	{
		m_pResource->Release();
	}

	if (m_pShaderResourceView)
	{
		m_pShaderResourceView->Release();
	}
}
ID3D11Texture2D* DirectX_Texture::GetResource() const
{
	return m_pResource;
}
ID3D11ShaderResourceView* DirectX_Texture::GetShaderResourceView() const
{
	return m_pShaderResourceView;
}

namespace dae
{
	Software_Texture::Software_Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Software_Texture::~Software_Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Software_Texture* Software_Texture::LoadFromFile(const std::string& path)
	{
		return new Software_Texture{ IMG_Load(path.c_str()) };
	}

	ColorRGB Software_Texture::Sample(const Vector2& uv) const
	{
		Uint8 r{};
		Uint8 g{};
		Uint8 b{};

		const int x{ static_cast<int>(uv.x * m_pSurface->w) };
		const int y{ static_cast<int>(uv.y * m_pSurface->h) };

		// Calculate the current pixelIndex on the texture
		const Uint32 pixelIndex{ m_pSurfacePixels[x + y * m_pSurface->w] };

		// Get the r g b values from the current pixel on the texture
		SDL_GetRGB(pixelIndex, m_pSurface->format, &r, &g, &b);

		const float maxColorValue{ 255.0f };

		return ColorRGB{ r / maxColorValue, g / maxColorValue, b / maxColorValue };
	}
}
