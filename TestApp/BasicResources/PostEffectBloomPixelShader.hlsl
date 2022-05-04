#include "PostEffectBloomHeader.hlsli"

float4 VerticalBlur(Texture2D<float4> tex, float2 uv, float dy, float filletrRadius, float bluerScale)
{
    float4 sum = float4(0,0,0,0);
    float wsum = 0;

    for (float y = -filletrRadius; y <= filletrRadius; y += 1.0f)
    {
        float4 sample = tex.Sample(smp, uv + float2(0, dy * y));

        float r = y * bluerScale;
        float w = exp(-r * r);


        sum += sample * w;
        wsum += w;
    }

    if (wsum > 0.0f)
    {
        sum /= wsum;
    }

    return sum;
}

float4 HorizontalBlur(Texture2D<float4> tex, float2 uv, float dx, float filletrRadius, float bluerScale)
{
    float4 sum = float4(0, 0, 0, 0);
    float wsum = 0;

    for (float x = -filletrRadius; x <= filletrRadius; x += 1.0f)
    {
        float4 sample = tex.Sample(smp, uv + float2(dx * x, 0));

        float r = x * bluerScale;
        float w = exp(-r * r);


        sum += sample * w;
        wsum += w;
    }

    if (wsum > 0.0f)
    {
        sum /= wsum;
    }

    return sum;
}


float4 VerticalBokehPS(Output input) : SV_TARGET
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    float dx = 1.0 / w;
    float dy = 1.0 / h;
    
    float4 bloomAccum = float4(0, 0, 0, 0);
    float2 uvSize = float2(1, 1);
    float2 uvOfst = float2(0, 0);
        
    bloomAccum += VerticalBlur(texShrinkHighLum0, input.uv, dy, 64, 0.08f);
           
    bloomAccum += VerticalBlur(texShrinkHighLum1, input.uv, dy, 8, 0.112f);
    
    bloomAccum += VerticalBlur(texShrinkHighLum2, input.uv, dy, 8, 0.112f);
    
    bloomAccum += VerticalBlur(texShrinkHighLum3, input.uv, dy, 8, 0.112f);

    return tex.Sample(smp,input.uv) + bloomAccum;

}

float getBrightness(float3 color)
{
    return max(color.r, max(color.g, color.b));
}



//Å@ÉuÉâÅ[
float4 BlurPS(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 1);
    
    return HorizontalBlur(texHighLum, input.uv, dx, 64, 0.08f);
}

float4 main(Output input) : SV_Target
{
    float w, h, level;
    tex.GetDimensions(0, w, h, level);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    
    float4 col = tex.Sample(smp, input.uv);

    float brightness = getBrightness(col.rgb);
    
    float contribution = max(0, brightness - 0.8f);
    contribution /= max(brightness, 0.00001);
        
    return col * contribution;
}