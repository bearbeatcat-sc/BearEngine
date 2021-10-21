
struct Payload
{
    float3 color;
};

struct MyAttribute
{
    float2 barys;
};

struct Vertex
{
    float3 pos;
    float3 normal;
    float3 uv;
};

struct SceneCB
{
    matrix mtxView;
    matrix mtxProj;
    matrix mtxViewInv;
    matrix mtxProjInv;
    float4 lightDirection;
    float4 lightColor;
    float4 ambientColor;
};

// GlobalRootSignature
RaytracingAccelerationStructure gRtScene : register(t0);
ConstantBuffer<SceneCB> gSceneParam : register(b0);

// Local  RayGen
RWTexture2D<float4> gOutput : register(u0);

// Local HitGroup
StructuredBuffer<uint> indexBuffer : register(t0,space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1,space1);


float3 linearToSrgb(float3 c)
{
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);

    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;

    return srgb;
}

inline float3 CalcBarycentrics(float2 barys)
{
    return float3(
        1.0 - barys.x - barys.y,
        barys.x,
        barys.y);
}

Vertex GetHitVertex(MyAttribute attrib)
{
    Vertex v = (Vertex) 0;
    float3 barycentrics = CalcBarycentrics(attrib.barys);
    uint vertexId = PrimitiveIndex() * 3; //Triangle List

    float weights[3] =
    {
        barycentrics.x, barycentrics.y, barycentrics.z
    };

    for (int i = 0; i < 3; ++i)
    {
        uint index = indexBuffer[vertexId + i];

    	// Weight？？
        float w = weights[i];
        v.pos += vertexBuffer[index].pos * w;
        v.normal += vertexBuffer[index].normal * w;
    }

    v.normal = normalize(v.normal);
    return v;
	
}

[shader("raygeneration")]
void rayGen()
{
	// ここらへん要、勉強
    uint2 lanchIndex = DispatchRaysIndex().xy;
    float2 dims = float2(DispatchRaysDimensions().xy);


    float2 d = (lanchIndex.xy + 0.5) / dims.xy * 2.0 - 1.0;
    float aspectRatio = dims.x / dims.y;

	
    matrix mtxViewInv = gSceneParam.mtxViewInv;
    matrix mtxProjInv = gSceneParam.mtxProjInv;

    RayDesc rayDesc;
    rayDesc.Origin = mul(mtxViewInv, float4(0, 0, 0, 1)).xyz;

    float4 target = mul(mtxProjInv, float4(d.x, -d.y, 1, 1));
    rayDesc.Direction = mul(mtxViewInv, float4(target.xyz, 0)).xyz;

    rayDesc.TMin = 0;
    rayDesc.TMax = 100000;

    Payload payload;
    payload.color = float3(0, 0, 0.5);
	
    TraceRay(gRtScene, RAY_FLAG_NONE, 0xff, 0, 1, 0, rayDesc, payload);

    float3 col = linearToSrgb(payload.color);
	

    gOutput[lanchIndex.xy] = float4(col, 1);
}


[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float3(0.4, 0.6, 0.2);
}

[shader("closesthit")]
void chs(inout Payload payload, in MyAttribute attribs)
{
    Vertex vtx = GetHitVertex(attribs);

    float3 lightDir = -normalize(gSceneParam.lightDirection.xyz);

    float nl = saturate(dot(vtx.normal, lightDir));

    float3 lightColor = gSceneParam.lightColor.xyz;
    float3 ambientColor = gSceneParam.ambientColor.xyz;
    float3 color = 0;

    color += lightColor * nl;
    color += ambientColor;

    payload.color = color;
}
