cbuffer cbuff0 : register(b0)
{
    matrix billboardMat;
    matrix vpMat;
};

struct VSOutput
{
    float4 pos : POSITION;
    float4 rotateAxisAndThita : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 scale : TEXCOORD2;
};

struct GSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

SamplerState smp : register(s0);
Texture2D<float4> tex : register(t0);
float4 main(GSOutput input) : SV_TARGET
{
    //float intensity = 0.5f - length(float2(0.5f, 0.5f) - input.uv);
    //intensity = clamp(intensity, 0.0f, 0.5f) * 2.0f;
    //return float4(input.color.xyz, intensity);
       
    return tex.Sample(smp, input.uv) * input.color;
}