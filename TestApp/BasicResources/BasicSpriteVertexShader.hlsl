cbuffer cbuff0 : register(b0)
{
    float4 color;
    matrix mat; // 3Dïœä∑çsóÒ
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput main( float4 pos : POSITION,float2 uv:TEXCOORD )
{
    VSOutput output;
    output.svpos = mul(mat, pos);
    output.uv = uv;
    return output;
}