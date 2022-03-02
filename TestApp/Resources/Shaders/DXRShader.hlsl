
struct Payload
{
    float3 color;
    int recursive;
};

struct ShadowPayload
{
    bool isHit;
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
    float4 albedo;
    float4 metallic;
    float roughness;
    float transmission;
    float refract;
    float pad;
};

struct SceneCB
{
    matrix mtxViewInv;
    matrix mtxProjInv;
    float4 lightDirection;
    float4 lightColor;
    float4 ambientColor;
    int pointLightCount;
};

struct PointLight
{
    float3 position;
    float padding;
    float4 color;
    float distance;
    float decay;
};

//反射光
struct ReflectedLight
{
	// ローカル照明
    float3 directDiffuse;
    float3 directSpecular;
};

// 入射光
struct IncidentLight
{
    float3 direction;
    float3 color;
    bool visible;
};


// GlobalSignature
RaytracingAccelerationStructure gRtScene : register(t0);
TextureCube<float4> gBackGround : register(t1);
StructuredBuffer<PointLight> gPointLights : register(t2);
ConstantBuffer<SceneCB> gSceneParam : register(b0);
SamplerState gSampler : register(s0);

// Local  RayGen
RWTexture2D<float4> gOutput : register(u0);

// Local HitGroup
StructuredBuffer<uint> indexBuffer : register(t0, space1);
StructuredBuffer<Vertex> vertexBuffer : register(t1, space1);
Texture2D<float4> texture : register(t2, space1);
ConstantBuffer<Material> matBuffer : register(b0, space1);


static const float PI = 3.1415926f;
static const float EPSILON = 1e-6;


inline float lengthSqr(float3 vec)
{
    return (vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

inline float hash(float3 p)
{
    p = frac(p * 0.3183099 + 0.1f);
    p *= 17.0f;

    return frac(p.x * p.y * p.z * (p.x + p.y + p.z));
}

// フレネル反射
inline float shlinck(float cos,float ri)
{
    float r0 = pow((1.0f - ri) / (1.0f + ri),2);
    return r0 + (1.0f - r0) * pow(1.0f - cos, 5);
}

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

// recursive = 再帰回数
float3 Reflection(float3 vertexPos, float3 vertexNormal, int recursive, float roughness)
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

float3 Refraction(float3 vertexPos, float3 vertexNormal, int recursive,float roughness,float refractVal)
{
    float4x3 mtx = ObjectToWorld4x3();

	// ワールド座標系に変換
    float3 worldPos = mul(float4(vertexPos, 1), mtx);
    float3 worldNormal = mul(vertexNormal, (float3x3) mtx);
    worldNormal = normalize(worldNormal);
	
    float3 worldRayDir = normalize(WorldRayDirection());

	// 物体の内部からか、物体の表面かを判定する
    float nr = dot(worldNormal, worldRayDir);

    float3 refracted;

    if (nr < 0)
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

    if (length(refracted) < 0.01)
    {
        return Reflection(vertexPos, vertexNormal, recursive, roughness);
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

inline bool scatter(float3 worldPos, float3 worldNormal, float refractVal,int recursive,float3 dispatchRayIndex,in float3 color)
{
    float3 outward_normal;
    float3 worldRayDir = WorldRayDirection();
    float3 reflectDir = reflect(worldRayDir, worldNormal);
    float3 reflactDir = float3(1, 1, 1);
    float reflect_prob = 1;
    float cosine;
    float ni_over_nt = refractVal;
	
    RAY_FLAG flags = RAY_FLAG_NONE;
    uint rayMask = 0xFF;

    RayDesc rayDesc;
    rayDesc.Origin = worldPos;
    rayDesc.Direction = reflectDir;
    rayDesc.TMin = 0.001;
    rayDesc.TMax = 100000;

    Payload refractPayload;
    refractPayload.color = float3(0, 0, 0);
    refractPayload.recursive = recursive;



    return true;
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
        v.uv += vertexBuffer[index].uv * w;
    }

    v.normal = normalize(v.normal);
    return v;
	
}

// ライトの減衰率を計算
float calclLightIntensity(const in float lightDistance,const in float cutoffDistance,const in float decayExponent)
{
	if(decayExponent > 0.0f)
	{
        return pow(saturate(-lightDistance / cutoffDistance + 1.0f), decayExponent);
    }

    return 1.0f;
}

bool isLightInRange(const in float lightDistance,const in float cutoffDistance)
{
    return any(float2(cutoffDistance == 0.0, lightDistance < cutoffDistance));
}

// ポイントライトによる入射光を計算する
void GetPointLightIrradiance(const in PointLight pointLight, float3 position,inout IncidentLight light)
{
    float3 l = pointLight.position - position;

	// 入射方向
    light.direction = normalize(l);

    float lightDistance = length(l);

    if (isLightInRange(lightDistance, pointLight.distance))
    {
        light.color = pointLight.color;
        light.color *= calclLightIntensity(lightDistance, pointLight.distance, pointLight.decay);
        light.visible = true;
    }
    else
    {
        light.color = float3(0.0f, 0.0f, 0.0f);
        light.visible = false;
    }
}

// マイクロファセットのGGX分布関数
float D_GGX(float a, float dotNH)
{
    float a2 = a * a;

    float dotNH2 = dotNH * dotNH;

    float d = dotNH2 * (a2 - 1.0) + 1.0;
	
    return a2 / (PI * d * d);
}

// 幾何減衰項
float G_Smith_Shlinck_GGX(float a, float dotNV, float dotNL)
{
    float k = a * a * 0.5 + EPSILON;

    float gl = dotNL / (dotNL * (1.0 - k) + k);
    float gv = dotNV / (dotNV * (1.0 - k) + k);

    return gl * gv;
}

// Shlinkの近似式によるフレネル項
float3 F_Schlink(float3 specularColor, float3 h, float3 v)
{
    return (specularColor + (1.0 - specularColor) * pow(1.0 - saturate(dot(v, h)), 5.0));
}

float3 ShlinkFresnel(float u, float f0, float f90)
{
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float3 specularBRDF(IncidentLight incident_light, float3 normal, float3 specularColor, float roughness)
{
    float3 n = normal;
    float3 v = normalize(-WorldRayDirection());
    float3 l = incident_light.direction;

    float dotNL = saturate(dot(n, l));
    float dotNV = saturate(dot(n, v));

    float3 halfVec = normalize(l + v);

    float dotNH = saturate(dot(n, halfVec));
    //float dotVH = saturate(dot(v, halfVec));
    //float dotLV = saturate(dot(l, v));

	// マイクロファセットの分布関数
    float a = roughness * roughness;
    float d = D_GGX(a, dotNH);

    float g = G_Smith_Shlinck_GGX(a, dotNV, dotNL);

    float3 f = F_Schlink(specularColor, halfVec, v);

    return (f * (g * d)) / (4.0 * dotNL * dotNV + EPSILON);
	
}

// 正規化したLambert
float3 diffuseBRDF(float3 diffuseColor)
{
    return diffuseColor / PI;
}

float3 NormalizeDisneyDiffuse(float3 albedo, IncidentLight incident_light, float3 normal, float roughness)
{
    float3 n = normal;
    float3 v = normalize(-WorldRayDirection());
    float3 l = incident_light.direction;

    float3 halfVec = normalize(l + v);

    float dotLH = saturate(dot(l, halfVec));
    float dotNL = saturate(dot(n, l));
    float dotNV = saturate(dot(n, v));
	
    float energyBias = lerp(0.0f, 0.5f, roughness);
    float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);

    float fd90 = energyBias + 2.0f * dotLH * dotLH * roughness;

	// フレネル反射率をラフネスに応じて
    float fl = ShlinkFresnel(1.0, fd90, dotNL);
    float fv = ShlinkFresnel(1.0, fd90, dotNV);
	
    return (albedo * fl * fv * energyFactor) / PI;
}

float3 phongShading(float3 albedo, float4 _spec, float3 vertexNormal, float3 vertexPos)
{
	
    float3 worldNormal = mul(vertexNormal, (float3x3) ObjectToWorld4x3());
    //float3 worldPos = mul(float4(vertexPos, 1), ObjectToWorld4x3());
    float3 worldPos = WorldRayOrigin() + RayTCurrent() + WorldRayDirection();
	

    float diffuse = saturate(dot(-gSceneParam.lightDirection.xyz, worldNormal));
    float3 diffuseColor = diffuse * albedo;



    float3 ambientColor = gSceneParam.ambientColor;
	
    return diffuseColor;

}


void CalculateLight(in IncidentLight incident_light, float3 normal, float3 diffuseColor, float3 specularColor, float roughness, inout ReflectedLight reflected)
{
    float dotNL = saturate(dot(normal, incident_light.direction));

	// 放射輝度の計算
    float3 irradiance = dotNL * incident_light.color;

	// 放射輝度にCosを掛けて放射照度に変換
    irradiance *= PI;

    //float diffuse = saturate(dot(-gSceneParam.lightDirection.xyz, normal));
    //float3 _diffuseColor = diffuse * diffuseColor;

    reflected.directDiffuse += irradiance * NormalizeDisneyDiffuse(diffuseColor, incident_light, normal, roughness);
    reflected.directSpecular += irradiance * specularBRDF(incident_light, normal, specularColor, roughness);
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


    gOutput[lanchIndex.xy] = float4(col.rgb, 1);

}

bool ShotShadowRay(float3 origin, float3 direction)
{


    RayDesc rayDesc;
    rayDesc.Origin = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin = 0.1;
    rayDesc.TMax = 100000;

    ShadowPayload payload;
    payload.isHit = true;

    RAY_FLAG flags = RAY_FLAG_NONE;

	// 交差したときのシェーダーは呼び出さない
    flags |= RAY_FLAG_SKIP_CLOSEST_HIT_SHADER;

    uint rayMask = ~(0x88);

    TraceRay(
    gRtScene,
    flags,
    rayMask,
    0,
    1,
    1,
    rayDesc,
    payload
    );

    return payload.isHit;
}


bool CheckShadow(in float3 lightDir, in float3 worldPosition)
{
    return ShotShadowRay(worldPosition, lightDir);
}

float3 GetRandomSphere(int seed,float3 position)
{
    const float r1 = hash(position * seed);
    const float r2 = hash(position * seed * 4.0f);

    float z = sqrt(1.0f - r2);
    float phi = PI * 2 * r1;

    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);

    return float3(x, y, z);
}


[shader("miss")]
void miss(inout Payload payload)
{
    //payload.color = float3(0.2f, 0.2f, 0.2f);
    float3 color = gBackGround.SampleLevel(
    gSampler, WorldRayDirection(), 0.0).xyz;
	
    payload.color = color;
}

[shader("miss")]
void shadowMiss(inout ShadowPayload payload)
{
    payload.isHit = false;
}


[shader("anyhit")]
void anyHit(inout Payload payload,in MyAttribute attribs)
{
    Vertex vtx = GetHitVertex(attribs);
	
    float4 tex = texture.SampleLevel(gSampler, vtx.uv, 0.0f);

    if (tex.w <= 0.0f)
    {
        IgnoreHit();
    }


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
    uint3 dispatchRayIndex = DispatchRaysIndex();

    float3 worldNormal = normalize(mul(vtx.normal, (float3x3) ObjectToWorld4x3()));
    float3 worldPosition = mul(float4(vtx.pos, 1), ObjectToWorld4x3());

    float4 metallic = matBuffer.metallic;
    float4 albedo = matBuffer.albedo;
    float roughness = matBuffer.roughness;
    float refract = matBuffer.refract;
    float transmission = matBuffer.transmission;

    float4 tex = texture.SampleLevel(gSampler, vtx.uv, 0.0f);
    albedo *= tex;

    bool isRefract = (transmission < 1.0f) ? true : false;
    bool isReflection = (metallic.w > 0.0f) ? true : false;



    float3 diffuseColor = lerp(albedo.rgb, float3(0.0f, 0.0f, 0.0f), metallic.w);
    float3 specularColor = lerp(float3(0.04f, 0.04f, 0.04f), albedo.rgb, metallic.w);

    IncidentLight light;
	
    ReflectedLight reflected;
    reflected.directDiffuse = diffuseColor;
    reflected.directSpecular = specularColor;

    const int pointLightCount = gSceneParam.pointLightCount;


    // ポイントライトの計算

    for (int i = 0; i < pointLightCount; ++i)
    {
        GetPointLightIrradiance(gPointLights[i], worldPosition, light);

        if (light.visible)
        {
            CalculateLight(light, worldNormal, diffuseColor, specularColor, roughness, reflected);
        }
    }
	

	// ディレクショナルライトの計算

    light.direction = -gSceneParam.lightDirection.xyz;
    light.color = gSceneParam.lightColor;
    light.visible = true;

    CalculateLight(light, worldNormal, diffuseColor, specularColor, roughness, reflected);
	

    if (isRefract)
    {
        float3 reflractionColor = Refraction(vtx.pos, vtx.normal, payload.recursive, roughness, refract);
        payload.color = lerp(reflected.directDiffuse + (reflected.directSpecular), reflractionColor + (reflected.directSpecular), 1.0f - transmission);

        return;
    }

	if (isReflection)
    {
        float3 reflectionColor = Reflection(vtx.pos, vtx.normal, payload.recursive, roughness);
        reflectionColor = lerp(float3(0.04f, 0.04f, 0.04f), reflectionColor.rgb, metallic.w);

        bool isInShadow = false;
        float3 shadowRayDir = -gSceneParam.lightDirection;
        isInShadow = CheckShadow(shadowRayDir, worldPosition);

        for (int i = 0; i < pointLightCount; ++i)
        {        	
            shadowRayDir = normalize(gPointLights[i].position.xyz - worldPosition.xyz);
            isInShadow = CheckShadow(shadowRayDir, worldPosition);
        }
			
        payload.color = reflected.directDiffuse + (reflected.directSpecular * reflectionColor);

        if (isInShadow)
        {
            payload.color.rgb *= 0.5;
        }
    	
        return;
    }

    bool isInShadow = false;
    float3 shadowRayDir = -gSceneParam.lightDirection;
    isInShadow = CheckShadow(shadowRayDir, worldPosition);

    for (int i = 0; i < pointLightCount; ++i)
    {
        shadowRayDir = normalize(gPointLights[i].position.xyz - worldPosition.xyz);
        isInShadow = CheckShadow(shadowRayDir, worldPosition);
    }
			
    payload.color = reflected.directDiffuse + reflected.directSpecular;

    if (isInShadow)
    {
        payload.color.rgb *= 0.5;
    }

}

