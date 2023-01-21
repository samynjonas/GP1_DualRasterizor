#pragma once
#include "Textures.h"
using namespace dae;

class effect
{
public:
	effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~effect();

	ID3DX11EffectTechnique* GetTechnique() const
	{
		return m_pTechnique;
	}
	ID3DX11Effect* GetEffect() const
	{
		return m_pEffect;
	}

	void SetWorldViewProjectionMatrix(const Matrix& matrix);
	virtual void SetInverseViewMatrix(const Matrix& matrix) {};
	virtual void SetWorldMatrix(const Matrix& matrix) {};

	void SetDiffuseMap(Textures* pTexture);

	void CycleFilteringMethod();

protected:
	enum class FilteringMethod
	{
		POINT,
		LINEAR,
		ANISOTROPIC
	};

	FilteringMethod m_FilteringMethod{ FilteringMethod::POINT };

	ID3DX11Effect* m_pEffect;
	ID3DX11EffectTechnique* m_pTechnique;
	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable;
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;

	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags = 0;

#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
		result = D3DX11CompileEffectFromFile(assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob	);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
					ss << pErrors[i];

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader : FAILED TO CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}
		return pEffect;
	}
};

