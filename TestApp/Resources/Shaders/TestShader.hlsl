
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);

float3 linearToSrgb(float3 c)
{
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);

    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;

    return srgb;
}

struct Payload
{
    float3 color;
};



[shader("raygeneration")]
void rayGen()
{
	// Ç±Ç±ÇÁÇ÷ÇÒóvÅAï◊ã≠
    uint3 lanchIndex = DispatchRaysIndex();
    uint3 lanchDim = DispatchRaysDimensions();


    float2 crd = float2(lanchIndex.xy);
    float2 dims = float2(lanchDim.xy);

    float2 d = ((crd / dims) * 2.0f - 1.0f);
    float aspectRatio = dims.x / dims.y;

    RayDesc ray;
    ray.Origin = float3(0, 0, -2);
    ray.Direction = normalize(float3(d.x * aspectRatio, -d.y, 1));

    ray.TMin = 0;
    ray.TMax = 1000;

    Payload payload;
    TraceRay(gRtScene, 0, 0xff, 0, 0, 0,ray, payload);

    float3 col = linearToSrgb(payload.color);
	

    gOutput[lanchIndex.xy] = float4(col, 1);
}


[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float3(0.4, 0.6, 0.2);
}

[shader("closesthit")]
void chs(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    const float3 A = float3(1, 0, 0);
    const float3 B = float3(0, 1, 0);
    const float3 C = float3(0, 0, 1);

    payload.color = A * barycentrics.x + B * barycentrics.y + C * barycentrics.z;
	
    //payload.color = float3(0.4, 0.6, 0.2);

}
