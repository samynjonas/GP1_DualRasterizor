#pragma once
class DirectX_Texture final
{
public:
	DirectX_Texture(const std::string& path, ID3D11Device* pDevice);
	~DirectX_Texture();

	ID3D11Texture2D* GetResource() const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;

private:
	ID3D11Texture2D* m_pResource{};
	ID3D11ShaderResourceView* m_pShaderResourceView{};

};

namespace dae
{
	struct Vector2;

	class Software_Texture final
	{
	public:
		~Software_Texture();

		static Software_Texture* LoadFromFile(const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

	private:
		Software_Texture(SDL_Surface* pSurface);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}

