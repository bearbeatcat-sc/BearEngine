
struct Payload
{
    float3 color;
    int recursive;
};

struct MyAttribute
{
    float2 barys;
};

struct Vertex
{
    float3 pos;
    float3 normal;
    float2 uv;
};

struct Material
{
    float3 diffuseColor;
    float3 specularColor;
    float specularRoughness;
};

//struct Material
//{
//	// 今はテスト用
//    bool isReflect;
//    float3 test;
//};

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

// GlobalSignature
RaytracingAccelerationStructure gRtScene : register(t0);
ConstantBuffer<SceneCB> gSceneParam : register(b0);
TextureCube<float4> gBackGround : register(t1);
SamplerState gSampler : register(s0);

// Local  RayGen
RWTexture2D<float4> gOutput : register(u0);

// Local HitGroup
StructuredBuffer<uint> indexBuffer : register(t0, space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1, space1);
//StructuredBuffer<Material> matBuffer : register(t2, space1);


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

float3 phongShading(float3 vertexNormal)
{
    //float3 worldNormal = mul(vertexNormal, (float3x3) ObjectToWorld4x3());
    float3 worldNormal = vertexNormal;
    float3 worldRayDir = WorldRayDirection();

    float3 lightDir = -normalize(gSceneParam.lightDirection.xyz);

    float nl = saturate(dot(worldNormal, lightDir));
    float3 diffuse = clamp(dot(worldNormal, lightDir), 0.0f, 1.0f);
    float3 specular = pow(clamp(dot(vertexNormal, nl), 0.0f, 1.0f), 50.0f);

    float3 testColor = float3(1, 1, 1);
	
    float3 color = testColor * float4(diffuse, 1.0f) + float4(specular, 1.0f) + gSceneParam.ambientColor;

    return color;
}

// recursive = 再帰回数
float3 Reflection(float3 vertexPos, float3 vertexNormal, int recursive)
{
    float3 worldPos = mul(float4(vertexPos, 1), ObjectToWorld4x3());
    float3 worldNormal = mul(vertexNormal, (float3x3) ObjectToWorld4x3());
    float3 worldRayDir = WorldRayDirection();
    float3 reflectDir = reflect(worldRayDir, worldNormal);

    RAY_FLAG flags = RAY_FLAG_NONE;
    uint rayMask = 0xFF;

	// 衝突した頂点から反射するイメージ
    RayDesc rayDesc;
    rayDesc.Origin = worldPos;
    rayDesc.Direction = reflectDir;
    rayDesc.TMin = 0.01;
    rayDesc.TMax = 100000;

    Payload reflectPayload;
    reflectPayload.color = float3(0, 0, 0);
    reflectPayload.recursive = recursive;

	// 再帰
    TraceRay(
    gRtScene,
    flags,
    rayMask,
    0,
    1,
    0,
    rayDesc,
    reflectPayload);

    return reflectPayload.color;
}

inline bool checkRecursiveLimit(inout Payload payload)
{
    payload.recursive++;
    if (payload.recursive >= 15)
    {
        payload.color = float3(0, 0, 0);
        return true;
    }
    return false;
}

float3 Refraction(float3 vertexPos,float3 vertexNormal,int recursive)
{
    float4x3 mtx = ObjectToWorld4x3();

	// ワールド座標系に変換
    float3 worldPos = mul(float4(vertexPos, 1), mtx);
    float3 worldNormal = mul(vertexNormal, (float3x3) mtx);
    worldNormal = normalize(worldNormal);
	
    float3 worldRayDir = normalize(WorldRayDirection());

	// 屈折率 (水の相対屈折率）
    const float refractVal = 1.4;

	// 物体の内部からか、物体の表面かを判定する
    float nr = dot(worldNormal, worldRayDir);

    float3 refracted;

	if(nr < 0)
	{
		// (1.0 == 空気中)
        float eta = 1.0 / refractVal;

        refracted = refract(worldRayDir, worldNormal, eta);
    }
	else
	{
        float eta = refractVal / 1.0;

		// 内部から表面にレイが飛んでるので、法線が反対になる。
        refracted = refract(worldRayDir, -worldNormal, eta);
    }

	if(length(refracted) < 0.01)
	{
		// 
        return Reflection(vertexPos, vertexNormal, recursive);
    }

    RAY_FLAG flags = RAY_FLAG_NONE;
    uint rayMask = 0xFF;

    RayDesc rayDesc;
    rayDesc.Origin = worldPos;
    rayDesc.Direction = refracted;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 100000;

    Payload refractPayload;
    refractPayload.color = float3(0, 0, 0);
    refractPayload.recursive = recursive;

    TraceRay(
    gRtScene,
    flags,
    rayMask,
    0,
    1,
    0,
    rayDesc,
	refractPayload);

    return refractPayload.color;

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
    payload.color = float3(0, 0, 0.0);
    payload.recursive = 0;
	
    TraceRay(gRtScene, RAY_FLAG_NONE, 0xff, 0, 1, 0, rayDesc, payload);

    float3 col = linearToSrgb(payload.color);
    //float3 col = payload.color;
	

    gOutput[lanchIndex.xy] = float4(col, 1);
}


[shader("miss")]
void miss(inout Payload payload)
{
    //payload.color = float3(0.2f, 0.2f, 0.2f);
    payload.color = gBackGround.SampleLevel(
    gSampler, WorldRayDirection(), 0.0).xyz;
}

[shader("closesthit")]
void chs(inout Payload payload, in MyAttribute attribs)
{
    if (checkRecursiveLimit(payload))
    {
        return;
    }
	
    Vertex vtx = GetHitVertex(attribs);
    uint id = InstanceID();
	
    float3 lightDir = -normalize(gSceneParam.lightDirection.xyz);

    float nl = saturate(dot(vtx.normal, lightDir));

  //  bool flag = matBuffer[0].isReflect;

  //  if (flag == false)
  //  {
	    	
		//// 今回は完全に反射する
	float3 reflectionColor = Reflection(vtx.pos, vtx.normal, payload.recursive);
	//float3 refractColor = Refraction(vtx.pos, vtx.normal, payload.recursive);
  //      payload.color = reflectionColor;

  //      return;
  //  }



	


    float3 color = lerp(phongShading(vtx.normal), reflectionColor, 0.8f);
	
    payload.color = reflectionColor;
};

