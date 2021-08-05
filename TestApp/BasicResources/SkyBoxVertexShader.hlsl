cbuffer cbuff0 : register(b0)
{
    matrix mat; // 3D?��ϊ�?��s?��?��
    float4 color;
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float3 texcoord:TEXCOORD;
};

VSOutput main(float4 pos : POSITION,float3 normal: NORMAL,float2 uv : TEXCOORD)
{
    VSOutput output;
    output.svpos = mul(mat, pos);
    output.normal = mul(mat,normal);
    output.texcoord = pos;
    return output;
}