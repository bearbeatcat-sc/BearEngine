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

float4 shadowVS(float4 pos : POSITION,float4 normal : NORMAL,float2 uv : TEXCOORD) : SV_Position
{
    return mul(shadowMat, pos);
}

VSOutput main(float4 pos : POSITION,float3 normal: NORMAL,float2 uv : TEXCOORD)
{
   

    
    VSOutput output;
    output.svpos = mul(mul(viewProj, world), pos);
    output.normal = normalize(mul(world, float4(normal, 0)));
    output.tpos = mul(shadowMat, pos);
    output.uv = uv;    
    
    float4 wpos = mul(world, pos);
    
    // Shading
    float3 lightColor = float3(1, 1, 1);
    
    const float shinness = 4.0f;
    float3 _diffuse = dot(-lightDir.xyz, output.normal) * diffuse;
    float3 _ambinet = ambient;
    
    float3 eyeDir = normalize(eyePos.xyz - wpos.xyz);
    float3 reflect = normalize(lightColor + 2 * dot(-lightColor, normal) * normal);
    float3 _specular = pow(saturate(dot(reflect, eyeDir)), shinness) * specular;
    
    
    output.color.rgb = ((_ambinet + _diffuse + _specular) * lightColor) * color.rgb;
    output.color.a = color.a;
    
    return output;
}