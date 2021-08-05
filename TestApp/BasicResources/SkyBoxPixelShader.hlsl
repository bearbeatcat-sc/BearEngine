TextureCube tex:register(t0);
SamplerState smp:register(s0);

cbuffer cbuff0 : register(b0)
{
    matrix mat; // 3D?��ϊ�?��s?��?��
    float4 color;
    float3 eyePos;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float3 texcoord:TEXCOORD;
};

float4 main(VSOutput input) : SV_TARGET
{
    // float3 eyeDir = normalize(input.svpos.xyz - eyePos.xyz);
    // float3 _reflect = reflect(eyeDir.xyz,input.normal.xyz);

    return tex.Sample(smp, input.texcoord.xyz) * color;
}