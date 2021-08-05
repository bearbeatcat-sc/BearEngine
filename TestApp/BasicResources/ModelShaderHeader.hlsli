cbuffer cbuff0 : register(b0)
{
    matrix world;
    matrix viewProj;
    matrix shadowMat;
    float4 color;
    float4 eyePos;
    float time;
}

cbuffer cbuff1 : register(b1)
{
    float4 lightColor;
    float4 lightDir;
}

cbuffer Material : register(b2)
{
    float3 ambient : packoffset(c0);
    float3 diffuse : packoffset(c1);
    float3 specular : packoffset(c2);
}

cbuffer cbuff2 : register(b3)
{
    matrix cbm_world;
    matrix cbm_viewProj;
    matrix cbm_shadowMat;
    float4 cbm_color;
    float4 cbm_eyePos;
    float cbm_time;
}


struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 wpos : POSIITON;
    float4 tpos : TPOS;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};