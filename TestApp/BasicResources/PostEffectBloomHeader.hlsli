Texture2D<float4> tex : register(t0); // NormalColor
Texture2D<float4> texHighLum : register(t1); // High
Texture2D<float4> texShrinkHighLum0 : register(t2); // ShrinkHigh
Texture2D<float4> texShrinkHighLum1 : register(t3); // ShrinkHigh
Texture2D<float4> texShrinkHighLum2 : register(t4); // ShrinkHigh
Texture2D<float4> texShrinkHighLum3 : register(t5); // ShrinkHigh

SamplerState smp : register(s0);

cbuffer Weight : register(b0)
{
    float4 bkweights[2];
};

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float4 col : SV_Target0; // レンダーターゲット！
};