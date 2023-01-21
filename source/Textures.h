#pragma once
class Textures final
{
public:
	Textures(const std::string& path, ID3D11Device* pDevice);
	~Textures();

	ID3D11Texture2D* GetResource() const;
	ID3D11ShaderResourceView* GetShaderResourceView() const;

private:
	ID3D11Texture2D* m_pResource{};
	ID3D11ShaderResourceView* m_pShaderResourceView{};

};

