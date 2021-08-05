cbuffer cbuff0 : register(b0)
{
    matrix worldMat;
    matrix vpMat;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
};

float4 main(VSOutput input) : SV_TARGET
{
    return float4(1,1,1,1);
}