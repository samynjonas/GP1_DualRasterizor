//------------------------------------------------
// Globals
//------------------------------------------------
Texture2D gDiffuseMap    : DiffuseMap;
Texture2D gNormalMap     : NormalMap;
Texture2D gSpecularMap   : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float4x4 gWorldViewProj  : WorldViewProjection;
float4x4 gWorldMatrix    : World;
float4x4 gViewInverse    : ViewInverse;

float gPI = 3.141592653f;
float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

SamplerState samPoint : SampleState
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samLinear : SampleState
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};

SamplerState samAnisotropic : SampleState
{
    Filter = ANISOTROPIC;
    AddressU = Wrap; //or Mirror, Clamp, Border
    AddressV = Wrap; //or Mirror, Clamp, Border
};


RasterizerState gRasterizerState
{
    CullMode = none;
    FrontCounterClockwise = false; //default
};

BlendState gBlendState
{
    BlendEnable[0] = false;
    RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
    DepthEnable = true;
    DepthWriteMask = 1;
    DepthFunc = less;
    StencilEnable = false;
};


// -----------------------------------------------------
// Input/Output structs
// -----------------------------------------------------
struct VS_INPUT
{
    float3 Position : POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
    float3 Tangent  : TANGENT;
};

struct VS_OUTPUT
{
    float4 Position      : SV_POSITION;
    float4 WorldPosition : COLOR;
    float2 UV            : TEXCOORD;
    float3 Normal        : NORMAL;
    float3 Tangent       : TANGENT;
};


// -----------------------------------------------------
// Vertex Shader
// -----------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output     = (VS_OUTPUT)0;
    output.Position      = mul(float4(input.Position, 1.0f), gWorldViewProj);
    output.UV            = input.UV;
    output.Tangent       = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
    output.Normal        = mul(normalize(input.Normal),  (float3x3)gWorldMatrix);
    return output;
}


// -----------------------------------------------------
// BRDF
// -----------------------------------------------------
float4 Lambert(float kd, float4 cd)
{
    float4 rho = cd * kd;
    float4 lambert = rho / gPI;

    return lambert;
}

float Phong(float ks, float exp, float3 l, float3 v, float3 n)
{
    float3 reflectVec = reflect(l, n);
    const float angle = max(dot(reflectVec, v), 0.0f);
    const float phong = ks * pow((angle), exp);

    return phong;
}


// -----------------------------------------------------
// Pixel Shader
// -----------------------------------------------------
float4 PS_Phong(VS_OUTPUT input, SamplerState state) : SV_TARGET
{
    const float4    lambert = Lambert(1.f, gDiffuseMap.Sample(state, input.UV));
    const float     specularExp = gShininess * gGlossinessMap.Sample(state, input.UV).r;
    const float3    viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
    const float4    specular = gSpecularMap.Sample(state, input.UV) * Phong(1.0f, specularExp, -gLightDirection, viewDirection, input.Normal);
    const float     observedArea = max(dot(input.Normal, gLightDirection), 1.0f);

    return (lambert + specular) * observedArea * gLightIntensity;
}

float4 PS_POINT(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,samPoint);
}

float4 PS_LINEAR(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,samLinear);
}

float4 PS_ANISOTROPIC(VS_OUTPUT input) : SV_TARGET
{
    return PS_Phong(input,samAnisotropic);
}

// -----------------------------------------------------
// Technique (Actual shader)
// -----------------------------------------------------
technique11 DefaultTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_POINT()));
    }
}

technique11 PointFilteringTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_POINT()));
    }
}

technique11 LinearFilteringTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_LINEAR()));
    }
}

technique11 AnisotropicFilteringTechnique
{
    pass P0
    {
        SetRasterizerState(gRasterizerState);
        SetDepthStencilState(gDepthStencilState, 0);
        SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_ANISOTROPIC()));
    }
}