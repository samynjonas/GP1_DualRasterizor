#pragma once
#include "effect.h"

class Effect_Shaded final: public effect
{
public:
	Effect_Shaded(ID3D11Device* pDevice, const std::wstring& assetFile);
	~Effect_Shaded();

	void SetNormalMap(DirectX_Texture* pTexture);
	void SetSpeculareMap(DirectX_Texture* pTexture);
	void SetGlossinessMap(DirectX_Texture* pTexture);

	void SetInverseViewMatrix(const Matrix& matrix) override;
	void SetWorldMatrix(const Matrix& matrix) override;

private:
	ID3DX11EffectMatrixVariable* m_pMatInvViewVariable;
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable;

	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable;
};

