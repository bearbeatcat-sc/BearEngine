Texture2D<float> tex : register(t0); // depthMap
SamplerState smp : register(s0);

cbuffer GausBlurParams : register(b0)
{
    float4 bkweights[2];
}

cbuffer BilateralBluerParams : register(b1)
{
    float BluerScale;
    float BluerDepthFallOff;
    float FillterRadius;
}

struct Output
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

struct PixelOutput
{
    float col : SV_Target0; 
};