#include "ModelShaderHeader.hlsli"


float4 shadowVS(float4 pos : POSITION,float4 normal : NORMAL,float2 uv : TEXCOORD) : SV_Position
{
    return mul(shadowMat, pos);
}

VSOutput main(float4 pos : POSITION,float3 normal: NORMAL,float2 uv : TEXCOORD)
{
    VSOutput output;
    
    //matrix mat = mul(world, viewProj);  
    
    output.svpos = mul(mul(cbm_viewProj, cbm_world), pos);
    output.normal = normalize(mul(cbm_world, float4(normal, 0)));
    output.tpos = mul(cbm_shadowMat, pos);
    output.uv = uv;
    output.wpos = mul(cbm_world, pos);
    
    return output;
}