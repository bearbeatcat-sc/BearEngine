cbuffer cbuff0 : register(b0)
{
    matrix billboardMat;
    matrix vpMat;
};

struct WallBuffer
{
    float4 pos;
    float4 scale;
    float pressure;
};

struct VSInput
{
    uint id : SV_VertexID;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 pos : POSITION;
    float4 axisAndThita : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 scale : TEXCOORD2;
};

struct GSOutput
{
    float4 svpos: SV_POSITION;
    float2 uv:TEXCOORD;
    float4 color : COLOR;
};

StructuredBuffer<WallBuffer> g_bufPosVelo;
VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = g_bufPosVelo[input.id].pos;
    output.axisAndThita = float4(0,0,0,1);
    output.scale = g_bufPosVelo[input.id].scale;
    output.color = float4(1, 0, 0, 1);

    return output;
}