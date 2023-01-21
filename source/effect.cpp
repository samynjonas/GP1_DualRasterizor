#include "pch.h"
#include "effect.h"

effect::effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);

	m_pTechnique = m_pEffect->GetTechniqueByName("PointFilteringTechnique");
	if (!m_pTechnique->IsValid())
		std::wcout << L"Technique not valid\n";

	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"m_pMatWorldViewProjVariable not valid\n";	

	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"m_pDiffuseMapVariable not valid\n";

	m_pRasterizerDesc = m_pEffect->GetVariableByName("gRasterizerState")->AsRasterizer();
	if (!m_pRasterizerDesc->IsValid())
		std::wcout << L"m_pDiffuseMapVariable not valid\n";
}

effect::~effect()
{
	if (m_pRasterizerDesc)
	{
		m_pRasterizerDesc->Release();
	}

	if (m_pDiffuseMapVariable)
	{
		m_pDiffuseMapVariable->Release();
	}	

	if (m_pMatWorldViewProjVariable)
	{
		m_pMatWorldViewProjVariable->Release();
	}

	if (m_pTechnique)
	{
		m_pTechnique->Release();
	}

	if (m_pEffect)
	{
		m_pEffect->Release();
	}	
}

void effect::SetWorldViewProjectionMatrix(const Matrix& matrix)
{
	m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void effect::SetDiffuseMap(DirectX_Texture* pDiffuseTexture)
{
	if (m_pDiffuseMapVariable)
		m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetShaderResourceView());
}

void effect::CycleFilteringMethod()
{
	m_FilteringMethod = static_cast<FilteringMethod>((static_cast<int>(m_FilteringMethod) + 1) % (static_cast<int>(FilteringMethod::ANISOTROPIC) + 1));
	std::cout << "Filtering Method: ";

	switch (m_FilteringMethod)
	{
	case FilteringMethod::POINT:
		m_pTechnique = m_pEffect->GetTechniqueByName("PointFilteringTechnique");
		
		if (!m_pTechnique->IsValid()) 
			std::wcout << L"PointTechnique not valid\n";
		
		std::cout << "POINT\n";
		break;
	case FilteringMethod::LINEAR:
		m_pTechnique = m_pEffect->GetTechniqueByName("LinearFilteringTechnique");
		
		if (!m_pTechnique->IsValid()) 
			std::wcout << L"LinearTechnique not valid\n";
		
		std::cout << "LINEAR\n";
		break;
	case FilteringMethod::ANISOTROPIC:
		m_pTechnique = m_pEffect->GetTechniqueByName("AnisotropicFilteringTechnique");
		
		if (!m_pTechnique->IsValid())
			std::wcout << L"AnisotropicTechnique not valid\n";
		std::cout << "ANISOTROPIC\n";

		break;
	}
}

void effect::CycleCullMode()
{
	ID3D11RasterizerState* pState;
	m_pRasterizerDesc->GetRasterizerState(0, &pState);

	D3D11_RASTERIZER_DESC pRasterizerDesc;
	pState->GetDesc(&pRasterizerDesc);

	m_CullMode = static_cast<CullMode>((static_cast<int>(m_CullMode) + 1) % (static_cast<int>(CullMode::Back) + 1));
	std::cout << "Switched Cullmode to ";
	switch (m_CullMode)
	{
	case effect::CullMode::Off:
		std::cout << "Off\n";
		pRasterizerDesc.CullMode = D3D11_CULL_NONE;
		break;
	case effect::CullMode::Front:
		std::cout << "Front\n";
		pRasterizerDesc.CullMode = D3D11_CULL_FRONT;
		break;
	case effect::CullMode::Back:
		std::cout << "Back\n";
		pRasterizerDesc.CullMode = D3D11_CULL_BACK;
		break;
	}

	m_pRasterizerDesc->SetRasterizerState(0, pState);
}