cbuffer cbuff0 : register(b0)
{
    matrix billboard_matrix;
    matrix view_matrix;
    matrix projection_matrix;
    matrix vp_matrix;
    float3 near_Far;
};

struct VSOutput
{
    float4 pos : POSITION;
    float4 rotateAxisAndThita : TEXCOORD0;
    float4 color : TEXCOORD1;
    float4 scale : TEXCOORD2;
};

struct GSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    float4 scale : TEXCOORD1;
    float3 pos : TECOORD2;
};

struct PSOUTPUT
{
    float tickness : SV_TARGET0;
    float depth : SV_TARGET1;
};

static const float size = 0.001f;

float linearizeDepth(float camera_Far,float camera_Near,float depth)
{

    
    float nearToFar = camera_Far - camera_Near;

    return 1.0f / ((1.0f - camera_Far / camera_Near) * depth + (camera_Far / camera_Near));
    
    //return (2 * camera_Near * camera_Far) / (camera_Far + camera_Near - (2.0f * depth - 1.0f) * (camera_Far - camera_Near));
    //return camera_Near * camera_Far / (depth * (camera_Near - camera_Far) + camera_Far);
    
    //return (1.0f - ((camera_Far / camera_Near) / camera_Far) * depth) / (depth * ((1.0f - camera_Far / camera_Near) / camera_Far));

}



SamplerState smp : register(s0);
Texture2D<float4> tex : register(t0);
PSOUTPUT main(GSOutput input)
{
    //float intensity = 0.5f - length(float2(0.5f, 0.5f) - input.uv);
    //intensity = clamp(intensity, 0.0f, 0.5f) * 2.0f;
    //return float4(input.color.xyz, intensity);
    
    PSOUTPUT output;
    
    // Depthだけ書き出し
    float3 N;
    N.xy = input.uv * 2.0f - 1.0f;
    float r2 = dot(N.xy, N.xy);
    
    if (r2 >= 1.0f)
        discard;
    
    
    
    // 仮
    //float scale = input.scale ;
    
    N.z = sqrt(1.0f - r2);
    
    // ここのScale値で深度に書き込まれる法線の模様が変わる
    float3 pixelPos = float3(input.pos + N * input.scale.x);
    float4 clipSpacePos = mul(projection_matrix, float4(pixelPos, 1.0f));
    //float flagDepth = linearizeDepth(near_Far.y, near_Far.x, (clipSpacePos.z / clipSpacePos.w) * 0.5f + 0.5f);
    //float flagDepth = linearizeDepth(near_Far.y, near_Far.x,(clipSpacePos.z / clipSpacePos.w ));
    //float flagDepth = linearizeDepth(near_Far.y, near_Far.x, clipSpacePos.z / clipSpacePos.w);
    
    float flagDepth = linearizeDepth(near_Far.y, near_Far.x, (clipSpacePos.z / clipSpacePos.w));
    //float flagDepth = clipSpacePos.z / clipSpacePos.w;
    //flagDepth = flagDepth * 0.5f + 0.5f;

       
    

    
    //return tex.Sample(smp, input.uv) * input.color;
    
    
    
    //float thickness = exp(-r2 * 0.2f);
    //float thickness = N.z * 0.001f;

    //float4 flagColor = float4(diffuse * float3(0.0f, 0.3f, 1.0f), 1.0f);
    //float4 flagColor = float4(diffuse * float3(1.0f, 1.0f, 1.0f), 1.0f);
    output.depth = flagDepth;
    
    // tickness
    
    output.tickness = exp(-r2 * 0.2f);
    //output.tickness = exp(-r2 * 0.2f);
    //output.tickness = thickness;
    return output;
    //return thickness;
    //float fragColor = pixelPos.z;
    
    //return float4(fragColor, fragColor, fragColor, 1.0f);

}