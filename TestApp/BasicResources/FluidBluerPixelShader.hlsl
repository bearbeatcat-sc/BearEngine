#include "FluidBluerHeader.hlsli"


//static float BluerScale = 0.16f;
//static float blurDepthFallOff = 1.2f;
//static float FillterRadius = 12.0f;

float VerticalBilateralBluer(Texture2D<float> tex, SamplerState smp, float2 uv, float dx, float dy)
{
    float ret = 0.0f;
    
    float depth = tex.Sample(smp, uv);
    
    ret += bkweights[0] * tex.Sample(smp, uv);
    
    for (int i = 1; i < 8; ++i)
    {
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(i * dx, i * dy));
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(-i * dx, -i * dy));
        
        float sample0 = tex.Sample(smp, uv + float2(i * dx, 0));
        float sample1 = tex.Sample(smp, uv + float2(-i * dx, 0));
        
        float r2 = (sample0 - depth) * BluerDepthFallOff;
        float g = exp(-r2 * r2);
        
        ret += (bkweights[i >> 2][i % 4] * g) * tex.Sample(smp, uv + float2(i * dx, 0));
        
        r2 = (sample1 - depth) * BluerDepthFallOff;
        g = exp(-r2 * r2);
        
        ret += (bkweights[i >> 2][i % 4] * g) * tex.Sample(smp, uv + float2(-i * dx, 0));
    }

    return ret;
}

float HorizontalBilateralBluer(Texture2D<float> tex, SamplerState smp, float2 uv, float dx, float dy)
{
    float ret = 0.0f;
    
    float depth = tex.Sample(smp, uv);

    ret += bkweights[0] * tex.Sample(smp, uv);
    
    for (int i = 1; i < 8; ++i)
    {
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(i * dx, i * dy));
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(-i * dx, -i * dy));
        
        float sample0 = tex.Sample(smp, uv + float2(0, i * dy));
        float sample1 = tex.Sample(smp, uv + float2(0, i * dy));
        
        
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(0, i * dy));
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(0, -i * dy));
    }

    return ret;
}


// ガウシアンブラー
float VerticalBluer(Texture2D<float> tex, SamplerState smp, float2 uv, float dx, float dy)
{
    float ret = 0.0f;
        
    ret += bkweights[0] * tex.Sample(smp, uv);
    
    for (int i = 1; i < 8; ++i)
    {
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(i * dx, i * dy));
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(-i * dx, -i * dy));       
        
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(i * dx, 0));
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(-i * dx, 0));
    }

    return ret;
}

float HorizontalBluer(Texture2D<float> tex, SamplerState smp, float2 uv, float dx, float dy)
{
    float ret = 0.0f;
    
    
    ret += bkweights[0] * tex.Sample(smp, uv);
    
    for (int i = 1; i < 8; ++i)
    {
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(i * dx, i * dy));
        //ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(-i * dx, -i * dy));
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(0, i * dy));
        ret += bkweights[i >> 2][i % 4] * tex.Sample(smp, uv + float2(0, -i * dy));
    }

    return ret;
}


// 横のブラー
float VerticalBlur_Main(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 0);
    
    //return VerticalBilateralBluer(tex, smp, input.uv, dx, dy);
        
   
    float2 uv = input.uv;

    float depth = tex.Sample(smp, uv);
	
    float sum = 0;
    float wsum = 0;

    for (float x = -FillterRadius; x <= FillterRadius; x += 1.0f)
    {
        float sample = tex.Sample(smp, uv + float2(dx * x, 0));

        float r = x * BluerScale;
        float w = exp(-r * r);

    	// 深度値によってブラーの強さを減衰する
        // (1 - 0 ) なら1 * power になる。
        //float r2 = (sample - depth) * BluerDepthFallOff;
        //float g = exp(-r2 * r2);
        
        float r2 = (sample - depth);
        float g = exp(-(r2 * r2) / (2 * BluerDepthFallOff * BluerDepthFallOff));


        sum += sample * w * g;
        wsum += w * g;
    }

    if (wsum > 0.0f)
    {
        sum /= wsum;
    }

    return sum;

}

// 縦のブラー
float HorizontalBlur_Main(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 0);

    
    //return HorizontalBilateralBluer(tex, smp, input.uv, dx, dy);
    


    float2 uv = input.uv;
    float depth = tex.Sample(smp, uv);
	
    float sum = 0;
    float wsum = 0;
    
    for (float y = -FillterRadius; y <= FillterRadius; y += 1.0f)
    {
        float sample = tex.Sample(smp, uv + float2(0, dy * y));

        float r = y * BluerScale;
        float w = exp(-r * r);

    	// 深度値によってブラーの強さを減衰する
        //float r2 = (sample - depth) * BluerDepthFallOff;
        //float g = exp(-r2 * r2);
        
        float r2 = (sample - depth);
        float g = exp(-(r2 * r2) / (2 * BluerDepthFallOff * BluerDepthFallOff));

        sum += sample * w * g;
        wsum += w * g;
    }

    if (wsum > 0.0f)
    {
        sum /= wsum;
    }

    return sum;

}

float VerticalGausBlur_Main(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 0);
    
    return VerticalBluer(tex, smp, input.uv, dx, dy);
}

// 縦のブラー
float HorizontalGausBlur_Main(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
       
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 0);
    
    return HorizontalBluer(tex, smp, input.uv, dx, dy);
    

}