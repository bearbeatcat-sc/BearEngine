Texture2D<float4> tex : register(t0); // NormalColor
Texture2D<float4> dofTex0 : register(t1); // DOFTex0
Texture2D<float4> dofTex1 : register(t2); // DofTex1
Texture2D<float> depthTexture : register(t3); // DepthTexture

SamplerState smp : register(s0);

cbuffer cbuff0 : register(b0)
{
    float Pint;
}

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 tex0 : SV_Target0;
    float4 tex1 : SV_Target0;
};