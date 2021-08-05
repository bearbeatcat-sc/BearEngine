Texture2D<float4> tex : register(t0); 
SamplerState smp : register(s0);

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 col : SV_Target0; // レンダーターゲット！
};