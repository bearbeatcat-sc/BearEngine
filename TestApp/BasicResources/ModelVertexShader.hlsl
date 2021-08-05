#include "ModelShaderHeader.hlsli"


float4 shadowVS(float4 pos : POSITION,float4 normal : NORMAL,float2 uv : TEXCOORD) : SV_Position
{
    return mul(shadowMat, pos);
}

VSOutput main(float4 pos : POSITION,float3 normal: NORMAL,float2 uv : TEXCOORD)
{
    VSOutput output;
    
    //matrix mat = mul(world, viewProj);  
    
    output.svpos = mul(mul(viewProj, world), pos);
    output.normal = normalize(mul(world, float4(normal, 0)));
    output.tpos = mul(shadowMat, pos);
    output.uv = uv;
    output.wpos = mul(world, pos);
    
    return output;
}