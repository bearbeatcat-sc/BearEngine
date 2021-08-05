cbuffer cbuff0 : register(b0)
{
    matrix billboardMat;
    matrix vpMat;
};

struct PosVelo
{
    float4 pos;
    float4 velo;
    float4 rotateAxis;
    float4 color;
    float4 scale;        
    bool activeFlag;
    float generateTime;
    float lifeTime;
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

StructuredBuffer<PosVelo> g_bufPosVelo;
VSOutput main(VSInput input)
{
    VSOutput output;
    output.pos = g_bufPosVelo[input.id].pos;
    output.axisAndThita = g_bufPosVelo[input.id].rotateAxis;
    output.scale = g_bufPosVelo[input.id].scale;
    output.color = g_bufPosVelo[input.id].color;

    return output;
}