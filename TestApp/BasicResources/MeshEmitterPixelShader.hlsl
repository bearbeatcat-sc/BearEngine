cbuffer cbuff0 : register(b0)
{
    matrix mat;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    //float4 scale : TEXCOORD2;
};

SamplerState smp : register(s0);
Texture2D<float4> tex : register(t0);
float4 main(VSOutput input) : SV_TARGET
{
    //float intensity = 0.5f - length(float2(0.5f, 0.5f) - input.uv);
    //intensity = clamp(intensity, 0.0f, 0.5f) * 2.0f;
    //return float4(input.color.xyz, intensity);
       
    return input.color * tex.Sample(smp, input.uv);
}