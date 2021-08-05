Texture2D<float> depthMap : register(t0); // depthMap
Texture2D<float> thicknessMap : register(t1); // thicknessMap
Texture2D<float4> backTexture : register(t2); // backTexture
TextureCube cubeMapTexture : register(t3);  // cubemap
Texture2D<float4> wallTexture : register(t4); // WallTexture

SamplerState smp : register(s0);

struct Result
{
    int wallPixelCount;
    int wallOnInkCount;
};

RWStructuredBuffer<Result> resultBuffer : register(u1);

cbuffer cbuff0 : register(b0)
{
    matrix Inverse_ProjectMat;
    matrix Inverse_ViewMat;
    matrix proj;
    matrix view;

}

cbuffer RenderingParams : register(b1)
{
    float4 FluidColor;
    float4 SpecularLight;
    float4 AmbientLight;
    float4 DiffuseLight;
    float4 lightDir;
    float4 eyePos;
    float clampDepth;
}

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 col : SV_Target0; // レンダーターゲット！
};