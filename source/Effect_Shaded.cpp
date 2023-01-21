#include "pch.h"
#include "Effect_Shaded.h"

Effect_Shaded::Effect_Shaded(ID3D11Device* pDevice, const std::wstring& assetFile)
	: effect(pDevice, assetFile)
{
	m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
	if (!m_pMatInvViewVariable->IsValid())
		std::wcout << L"m_pMatInvViewMatrixVariable not valid\n";

	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::wcout << L"m_pMatWorldVariable not valid\n";

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"m_pNormalMapVariable not valid\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"m_pSpecularMapVariable not valid\n";

	m_pGlossinessMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
	if (!m_pGlossinessMapVariable->IsValid())
		std::wcout << L"m_pGlossinessMapVariable not valid\n";
}

Effect_Shaded::~Effect_Shaded()
{
	if (m_pGlossinessMapVariable)
	{
		m_pGlossinessMapVariable->Release();
	}

	if (m_pSpecularMapVariable)
	{
		m_pSpecularMapVariable->Release();
	}

	if (m_pNormalMapVariable)
	{
		m_pNormalMapVariable->Release();
	}

	if (m_pMatWorldVariable)
	{
		m_pMatWorldVariable->Release();
	}

	if (m_pMatInvViewVariable)
	{
		m_pMatInvViewVariable->Release();
	}
}

void Effect_Shaded::SetInverseViewMatrix(const Matrix& matrix)
{
	m_pMatInvViewVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}
void Effect_Shaded::SetWorldMatrix(const Matrix& matrix)
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&matrix));
}

void Effect_Shaded::SetNormalMap(Textures* pTexture)
{
	if (m_pNormalMapVariable)
		m_pNormalMapVariable->SetResource(pTexture->GetShaderResourceView());
}
void Effect_Shaded::SetSpeculareMap(Textures* pTexture)
{
	if (m_pSpecularMapVariable)
		m_pSpecularMapVariable->SetResource(pTexture->GetShaderResourceView());
}
void Effect_Shaded::SetGlossinessMap(Textures* pTexture)
{
	if (m_pGlossinessMapVariable)
		m_pGlossinessMapVariable->SetResource(pTexture->GetShaderResourceView());
}