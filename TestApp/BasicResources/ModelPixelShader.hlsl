Texture2D<float4> tex:register(t0);
Texture2D<float> lightDepthTex : register(t1);
SamplerState smp:register(s0);
SamplerComparisonState shadowSmp : register(s1);

#include "ModelShaderHeader.hlsli"


float4 main(VSOutput input) : SV_TARGET
{    
    // 頂点
    //float3 P = input.wpos;
    
    //// ノーマル
    //float3 N = normalize(input.normal);
    
    
   
    //float3 L = normalize(lightDir.xyz - P); //　ローカル座標系のでのライトベクトル
    
    //// Diffuseの計算
    //float diffuseLight = max(dot(L, N), 0);    
    //float3 _diffuse = diffuse.xyz * lightColor * diffuseLight;

    //float3 V = normalize(eyePos.xyz - P);
    //float3 H = normalize(L + V); //　ハーフベクトル
    //float3 specularLight = pow(max(dot(N, H), 0), 120);
    //float3 _specular = specular.xyz * lightColor * specularLight;
    
    //float4 texColor = tex.Sample(smp, input.uv);
    //float3 materialColor = ambient.xyz + _diffuse + _specular;
    
    float4 texColor = tex.Sample(smp, input.uv);
    const float shiiness = 4.0f;
    
    float3 eyeDir = normalize(eyePos - input.wpos.xyz);
    float3 dotlightNormal = dot(-lightDir, input.normal);
    float3 reflect = normalize(lightDir + 2 * dotlightNormal * input.normal);
    
    float3 _ambient = ambient;
    float3 _diffuse = dotlightNormal * diffuse;
    float3 _specular = pow(saturate(dot(reflect, eyeDir)), shiiness) * specular;

    float3 materialColor = (_ambient + _diffuse + _specular) * lightColor;
    
    float4 ret = float4(texColor.rgb * materialColor.rgb, 1.0f) * color;
    
    //SelfShadow    
    float shadowWeight = 1.0f;
    // テクスチャ座標へ変換
    float3 posFromLightVP = input.tpos.xyz / input.tpos.w;
    
    float2 shadowUV = (posFromLightVP + float2(1, -1)) * float2(0.5, -0.5);
    
    float depthFromLight = lightDepthTex.SampleCmp(
    shadowSmp,
    shadowUV,
    posFromLightVP.z - 0.005f);
    
    shadowWeight = lerp(0.5f, 1.0f, depthFromLight);
    
    return float4(ret.rgb * shadowWeight, ret.a);
}