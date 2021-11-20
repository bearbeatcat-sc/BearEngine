#include "PeraPolygonDrawShaderHeader.hlsli"


float4 main(Output input) : SV_Target
{

    float4 col = tex.Sample(smp, input.uv);
   
    return col;
}

float4 RaytracingBlendPS(Output input) : SV_Target
{
    
    float4 col = tex.Sample(smp, input.uv);
    
    return col;

}