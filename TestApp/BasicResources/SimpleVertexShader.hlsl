cbuffer cbuff0 : register(b0)
{
    matrix worldMat;
    matrix vpMat;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
};

VSOutput main(float4 pos : POSITION)
{
    VSOutput output;
    output.svpos = mul(mul(vpMat, worldMat), pos);
    return output;
}