#include "PostEffectDOFHeader.hlsli"

float linearizeDepth(float depth)
{
    // 仮
    float near = 0.3f;
    float far = 1000.0f;
    
    //float nearToFar = camera_Far - camera_Near;
    //return 1.0f / ((1.0f - near / far) * depth + (near / far));
    
    
    //return 1.0f / ((-1.0f + near / far) * depth + 1.0f);

    //return near * far / (far + depth * (far - near));
	
    return (2 * near) / (far + near - depth * (far - near));
}

float gausian(float2 uv, float2 colorRange, float sigma)
{
    float r = distance(uv, colorRange + uv);
    float weight = exp(-(r * r) / (2.0f * sigma * sigma));

    return weight;
}

float4 VerticalBlur(Texture2D<float4> tex, float2 uv, float dy, float filletrRadius, float bluerScale)
{
    float4 sum = 0;
    float wsum = 0;

    for (float y = -filletrRadius; y <= filletrRadius; y += 0.01f)
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
    float4 sum = 0;
    float wsum = 0;

    for (float x = -filletrRadius; x <= filletrRadius; x += 0.01f)
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

float4 gausblur(Texture2D<float4> tex, float sigma, float2 uv)
{
    float4 totalWeight = 0;
    float2 pick = float2(0, 0);
    
    float4 color = float4(0, 0, 0, 0);
    float pickRange = sigma;
    
    [loop]
    for (pick.y = -pickRange; pick.y <= pickRange; pick.y += 0.01f)
    {
        [loop]
        for (pick.x = -pickRange; pick.x <= pickRange; pick.x += 0.01f)
        {
            float weight = gausian(uv, pick, pickRange);
            color += tex.Sample(smp, uv + pick) * weight;
            totalWeight += weight;
        }

    }

    color = color / totalWeight;
    
    return color;
}

// 縦のブラーと合成を行う。
float4 DepthOfField(Output input) : SV_TARGET
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    float dx = 1.0 / w;
    float dy = 1.0 / h;
	
    //Get5x5GaussianBlur(texHighLum, smp, input.uv, dx, dy, float4(uvOfst, uvOfst + uvSize)) +
    //saturate(bloomAccum);


    
    
    
    float4 dof1 = VerticalBlur(dofTex0,input.uv,dy,4,0.03f);
    float4 dof2 = VerticalBlur(dofTex1, input.uv, dy, 4, 0.03f);
    
    //return dof2;
    //return dof1;
           
    float depth = linearizeDepth(depthTexture.Sample(smp, input.uv));
    		
    
    float sub = abs(depth - Pint);
    
    float far = smoothstep(0.3f, 0.6f, sub);
    float near = 1.0f - smoothstep(0.0f, 0.3f, sub);

    
    //if (coef >= 0.8f)
    //{
    //    return float4(1, 0, 0, 1);
    //}
    //return float4(0, 0, 1, 1);

    float4 scene = tex.Sample(smp, input.uv);
    		
    return lerp(lerp(scene, dof1, 1 - far - near), dof2, far);

}

float getBrightness(float3 color)
{
    return max(color.r, max(color.g, color.b));
}



//　横のブラー
PixelOutput Horizontal(Output input) : SV_Target
{
    float w, h, miplevels;
    tex.GetDimensions(0, w, h, miplevels);
    
    float dx = 1.0f / w;
    float dy = 1.0f / h;
    float2 uvSize = float2(0.5f, 0.5f);
    float2 uvOfst = float2(0, 0);
    
    float4 ret = float4(0, 0, 0, 0);

    PixelOutput pixelOutput;
    pixelOutput.tex0 = HorizontalBlur(tex, input.uv, dx, 4, 0.03f);
    pixelOutput.tex1 = HorizontalBlur(tex, input.uv, dx, 4, 0.03f);
    
    return pixelOutput;
}

