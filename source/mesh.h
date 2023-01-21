#pragma once
#include "DataTypes.h"
#include "Effect_Shaded.h"
#include <vector>

using namespace dae;

class mesh final
{
public:
	mesh(ID3D11Device* pDevice, std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, effect* pEffect);
	~mesh();

	void Render(ID3D11DeviceContext* pDeviceContext);

	void UpdateMatrices(const Matrix& WorldViewProjection, const Matrix& invView);

	void CycleFilteringMethod();

	void RotateX(float angle);
	void RotateY(float angle);
	void RotateZ(float angle);

private:
	effect* m_pEffect{};	

	Matrix m_TranslationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_RotationMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };
	Matrix m_ScaleMatrix{ Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, Vector3::Zero };

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};

	uint32_t m_NumIndices{};
	ID3D11Buffer* m_pIndexBuffer{};
};

