Texture2D<float4> tex:register(t0);
Texture2D<float> lightDepthTex : register(t1);
SamplerState smp:register(s0);
SamplerComparisonState shadowSmp : register(s1);

cbuffer cbuff0 : register(b0)
{
    matrix world;
    matrix viewProj;
    matrix shadowMat;
    float4 color;
    float4 eyePos;
    float time;
}

cbuffer cbuff1 : register(b1)
{
    float4 lightColor;
    float4 lightDir;
}

cbuffer Material : register(b2)
{
    float3 ambient : packoffset(c0);
    float3 diffuse : packoffset(c1);
    float3 specular : packoffset(c2);
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float4 tpos : TPOS;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(VSOutput input) : SV_TARGET
{
    //float3 P = input.svpos.xyz;
    //float3 N = normalize(input.normal);
    
    //float3 lightColor = (1, 1, 1);
    
    //float3 L = normalize(lightPos - P); //　ローカル座標系のでのライトベクトル
    //float diffuseLight = max(dot(L, N), 0);
    //float3 _diffuse = diffuse * lightColor * diffuseLight;

    //float3 V = normalize(eyePos - P);
    //float3 H = normalize(L + V); //　ハーフベクトル
    //float3 specularLight = pow(max(dot(N, H), 0), 120);
    //float3 _specular = specular * lightColor * specularLight;
    
    float4 texColor = tex.Sample(smp, input.uv);
    //float3 materialColor = ambient + _diffuse + _specular;

    float4 ret = float4(texColor.rgb * input.color.rgb, input.color.a);
    
    return ret;

}