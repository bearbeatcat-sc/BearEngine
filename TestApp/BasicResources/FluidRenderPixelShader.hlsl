#include "FluidRenderHeader.hlsli"


// これは後で可変にする必要があるかも
//float MAP_WIDTH = 1920;
//float MAP_HEIGHT = 1080;

//float4 vBais = { 2 / 1920, 4 / 1920, 6 / 1920, 8 / 1920 };
//float4 hBais = { 2 / 1080, 4 / 1080, 6 / 1080, 8 / 1080 };


float linearizeDepth(float camera_Far, float camera_Near, float depth)
{

    
    float nearToFar = camera_Far - camera_Near;

    return 1.0f / ((1.0f - camera_Far / camera_Near) * depth + (camera_Far / camera_Near));
    
    //return (2 * camera_Near * camera_Far) / (camera_Far + camera_Near - (2.0f * depth - 1.0f) * (camera_Far - camera_Near));
    //return camera_Near * camera_Far / (depth * (camera_Near - camera_Far) + camera_Far);
    
    //return (1.0f - ((camera_Far / camera_Near) / camera_Far) * depth) / (depth * ((1.0f - camera_Far / camera_Near) / camera_Far));

}


float GetZ(float x, float y)
{
    // 反転させることでZ値？？？
    return depthMap.Sample(smp, float2(x, y));
    
    //float near = 0.01f;
    //float far = 1000.0f;
    
    //float depth = depthMap.Sample(smp, float2(x, y));

    //return far - (depth * (far - near));
}

 //UV座標からワールド座標を計算
float3 uvToEye(float2 uv, float depth)
{
    // UV座標からProjection座標へ
    float2 pos = uv * 2.0f - 1.0f;
    //float2 pos = uv;

    
    float4 clipPos = float4(pos, depth, 1.0f);

    //float Near = 1.0f;
    //float Far = 1000.0f;

    //float a = Far / (Far - Near);
    //float b = Far * Near / (Near - Far);
    //float rd = b / (depth - a);
    
    
    // ビュー座標系に変換
    float4 viewPos = mul(Inverse_ProjectMat, clipPos);
    
    return viewPos.xyz / viewPos.w;

    //return float3(pos, -1.0f) * rd;
}

//float3 uvToEye(float2 uv,float depth)
//{
//    float camera_Far = 1000.0f;

//	// 視錐台での線形補間によってView位置
//    float3 frusutumRay = float3
//	(
//		lerp(camearaFrustum.x, camearaFrustum.y, uv.x),
//    	lerp(camearaFrustum.z, camearaFrustum.w, uv.y),
//		-camera_Far
//	);

//    return frusutumRay * depth;
//}

float3 getEyePos(float2 uv)
{
    return uvToEye(uv, GetZ(uv.x, uv.y));
}

float3 ViewPosition(float2 uv)
{
    float clip_z = 1.0f - depthMap.Sample(smp, uv).r;
    float clip_x = uv.x * 2.0f - 1.0f;
    float clip_y = 1.0f - uv.y * 2.0f;
    
    // UV座標を元にProjection座標での位置を算出
    float4 clip_p = float4(clip_x, clip_y, clip_z, 1.0f);
    
    // VPの逆行列でビュー座標に変換
    float4 view_p = mul(Inverse_ProjectMat, clip_p);
    
    return view_p.z / view_p.w;
}

float3 reconstructNormal(float2 uv, float3 vp11)
{
    // テクセルサイズ
    float2 ts = float2(1.0f / 1920.0f, 1.0f / 1080.0f);
    
    // UV座標の位置の近接ピクセルの位置を計算。
    float3 vp12 = ViewPosition(uv + ts * float2(0, 1));
    float3 vp10 = ViewPosition(uv + ts * float2(0, -1));
    float3 vp21 = ViewPosition(uv + ts * float2(1, 0));
    float3 vp01 = ViewPosition(uv + ts * float2(-1, 0));

    // 各位置から法線を計算する
    float3 dvpdx0 = vp11 - vp12;
    float3 dvpdx1 = vp10 - vp11;
    float3 dvpdy0 = vp11 - vp21;
    float3 dvpdy1 = vp01 - vp11;
    
    if (dot(dvpdx0, dvpdx0) > dot(dvpdx1, dvpdx1))
    {
        dvpdx0 = dvpdx1;
    }
    
    if (dot(dvpdy0, dvpdy0) > dot(dvpdy1, dvpdy1))
    {
        dvpdy0 = dvpdy1;
    }
    
    return normalize(cross(dvpdx0, dvpdy0));
}

// Schlickの近似を使ったフレネルの公式
float calcFresnel(float f0, float u)
{
    return f0 + (1.0f - f0) * pow(1.0f - u, 5);
}

bool rayIntersect(float z, float2 uv, float rayThickness)
{
    float sceneZ = GetZ(uv.x, uv.y);
    
    float dist = z - sceneZ;
    
    // 差分が0.0ならレイが衝突
    return dist < 0.0f && dist > -rayThickness;
}

// 最終的なレンダリング結果
float4 main(Output input) : SV_Target
{
    float2 uv = input.uv;
    
    float depth = GetZ(uv.x, uv.y);
    //float backDepth = linearizeDepth(1000.0f, 0.01f, backDepthTexture.Sample(smp, uv));
    float thickness = clamp(thicknessMap.Sample(smp, input.uv), 0.001f, 1.0f);
    float4 walltex = wallTexture.Sample(smp, input.uv);
    //float thickness = thicknessMap.Sample(smp, input.uv);

    if (walltex.r > 0
        || walltex.g > 0
        || walltex.b > 0)
    {
        resultBuffer[0].wallPixelCount += 1;
    }
	
    if (depth <= 0.0014f)
    {
        return backTexture.Sample(smp, input.uv);
    }   

    if (walltex.r > 0
        || walltex.g > 0
        || walltex.b > 0)
    {
        resultBuffer[0].wallOnInkCount += 1;
    }
	
    float w, h, mipLevels;
    float2 ts = float2(0, 0);
    depthMap.GetDimensions(0, w, h, mipLevels);
    
    //return float4(depth, depth, depth, 1);
    
    ts.x = 1.0f / w;
    ts.y = 1.0f / h;
    
    float3 posEye = uvToEye(input.uv, depth);
    
    float3 ddx = getEyePos(input.uv + float2(ts.x, 0)) - posEye;
    float3 ddy = getEyePos(input.uv + float2(0, ts.y)) - posEye;

    float3 ddx2 = posEye - getEyePos(input.uv - float2(ts.x, 0));
    float3 ddy2 = posEye - getEyePos(input.uv - float2(0, ts.y));

    if (abs(ddx.z) > abs(ddx2.z))
    {
        ddx = ddx2;
    }
    
    
    if (abs(ddy.z) > abs(ddy2.z))
    {
        ddy = ddy2;
    }
    
    float3 n = cross(ddx, ddy);
    n = normalize(n);
    n = normalize(mul(view, float4(n, 0.0f)));
    n = n * 0.5f + 0.5f;
                                  	
    float3 lightdir = normalize(lightDir.xyz);
    float3 worldPos = mul(Inverse_ViewMat, float4(posEye, 1.0f)).xyz;
    float3 viewDir = normalize(worldPos - eyePos.xyz);           
    
    float3 L = normalize(-lightdir.xyz - posEye);
    float3 V = normalize(eyePos.xyz - posEye);
    
    float3 halfVec = normalize(L + V);
    float Fresnel = calcFresnel(1.0f / 1.5f, dot(L, halfVec));

    float diffuse = clamp(dot(n, -lightdir), 0.0f, 1.0f);
        
    float specular = pow(clamp(dot(n, halfVec), 0.0f, 1.0f), 100.0f);
        
    float3 reflectionColor = cubeMapTexture.Sample(smp, reflect(-viewDir, n)).rgb;
    reflectionColor = lerp(float3(0.0f, 0.0f, 0.0f), reflectionColor, smoothstep(0.05f, 0.3f, worldPos.y));
    
    //if (depth <= 0.0001f)
    //{
    //    return lerp(backTexture.Sample(smp, input.uv), float4(lerp(diffuse * DiffuseLight.rgb, lerp(diffuse * DiffuseLight.rgb, reflectionColor, 0.2f), Fresnel)
    //+ specular * SpecularLight.rgb, 1), 0.1f);
    //    //return float4(0, 0, 0, 0);
    //}
    
    float3 diffuseColor = diffuse * DiffuseLight.rgb;
    
    //if(diffuse > 0.8f)
    //{
    //    diffuseColor = float3(1, 1, 1);
    //}
    
    //if (diffuse > 0.9f)
    //{
    //    diffuseColor = float3(0, 1, 1);
    //}
    
    return float4(lerp(diffuseColor, lerp(diffuseColor, reflectionColor, 0.2f), Fresnel)
    +specular * SpecularLight.rgb, 1);
	
    //float Fresnel = calcFresnel(0.02f, saturate(1.0f - dot(n, viewDir)));
    //float Fresnel = calcFresnel(0.02f, dot(L, halfVec));
    
    //thickness = thickness < 0.0001f ? 0.0f : thickness;

    float rayDist = 3.0f;
    
    //// 入射角
    float3 reflectVec = reflect(-viewDir, n);

    //float3 reflectionColor = cubeMapTexture.Sample(smp, reflect(-viewDir, n)).rgb;

    //bool intersect = false;
    //int iterationCount = 100;
    
    //float rayLength = ((posEye.z + reflectVec.z * rayDist) > -0.01f) ?
    //    (-0.01f - posEye.z) / reflectVec.z : rayDist;
    
    //float3 rayEnd = posEye + reflectVec * rayLength;
    
    //float3 q0 = posEye;
    //float3 q1 = rayEnd;
    
    //float3 delta = q1 - q0;
    //float3 deltaStep = delta / iterationCount;
    
    //float3 q = q0;
    //float2 p;
    //float2 hitPixel;
    
    
    //for (int i = 0; i < iterationCount; i++)
    //{
    //    if (i >= iterationCount - 1)
    //    {
    //        break;
    //    }
        
    //    if (intersect)
    //    {
    //        reflectionColor = backTexture.Sample(smp, hitPixel);
    //        reflectionColor *= cubeMapTexture.Sample(smp, reflect(-viewDir, n));
    //        break;
    //    }
        
    //    q += deltaStep;
    //    float4 clip = mul(proj, float4(q, 1.0f));
    //    p = clip.xy / clip.w;
        
    //    hitPixel = p.xy * 0.5f + 0.5f;
        
    //    intersect = rayIntersect(q.z, hitPixel, 0.1f);
    //}
    
    reflectionColor = lerp(float3(0.0f, 0.0f, 0.0f), reflectionColor, smoothstep(0.05f, 0.3f, worldPos.y));
                
    
    // 相対屈折率
    float ratio = 1.0f / 1.33f;
    float3 refraciton = refract(-viewDir, n, ratio);
    
    //rayLength = ((posEye.z + refraciton.z * rayDist) > -0.01f) ?
    //    (-0.01f - posEye.z) / refraciton.z : rayDist;
    
    float3 refractionPoint = refraciton * thickness;
        
    float4 clip = mul(proj, float4(refractionPoint, 1.0f));
    float2 p = clip.xy / clip.w;
        
    float2 hitPixel = p.xy * 0.5f + 0.5f;
    
    float3 refracationColor = backTexture.Sample(smp, hitPixel);
                     
    //intersect = false;
    
    //rayEnd = posEye + refraciton * rayLength;
    
    //q0 = posEye;
    //q1 = rayEnd;
    
    //delta = q1 - q0;
    //deltaStep = delta / iterationCount;
    
    //q = q0;
        
    //for (i = 0; i < iterationCount; i++)
    //{
    //    if (i >= iterationCount - 1)
    //    {
    //        break;
    //    }
        
    //    if (intersect)
    //    {
    //        refracationColor = backTexture.Sample(smp, hitPixel);
    //        break;
    //    }
        
    //    q += deltaStep;
    //    float4 clip = mul(proj, float4(q, 1.0f));
    //    p = clip.xy / clip.w;
        
    //    hitPixel = p.xy * 0.5f + 0.5f;
        
    //    intersect = rayIntersect(q.z, hitPixel, 0.1f);
    //}
    
    float dotSpec = saturate(dot(reflectVec, lightdir) * 0.5f + 0.5f);

    float specularColor = (1.0f - Fresnel) * saturate(lightdir.y) * pow(dotSpec, 512.0f) * (2.0f * 1.8f + 0.2f);
    specularColor += specularColor * saturate(2.0f * -0.05f) * float3(0.2f, 0.2f, 0.2f);
    
    //return float4(refracationColor, 1);
    
    //return float4(reflectionColor, 1) + float4(specularColor, specularColor, specularColor, 1);
    return float4(lerp(refracationColor, reflectionColor, Fresnel), 1) + float4(specularColor, specularColor, specularColor, 1);

    //reflectionColor *= cubeMapTexture.Sample(smp, reflectVec).rgb;
    //reflectionColor = lerp(float3(0.0f, 0.0f, 0.0f), reflectionColor, smoothstep(0.05f, 0.3f, worldPos.y));

    
    
	
    //diffuse    
    // 球体のWorldPositionの位置に応じて暗くなる？
    //float3 diffuse = diff_power * worldPos.y * diffuseColor;
    
    //float thickness = thicknessMap.Sample(smp, input.uv);
    //float dist_power = 1.0f - thickness;
    
    ////float3 diffuse = (diffuseColor * dot(-lightdir.xyz, n)) * thickness;
    //diffuse = (1.0f - dist_power) * diffuse;
    


    
    //float Transparency = 1.0f;
    //Transparency *= 1.0f - (thickness * absorption);
    
    //float dotL = dot(lightdir, n);
    //return float4(depth, depth, depth, 1.0);
    
    //float3 diffuseColor = float3(0, 0, 0);
    //float diffuseFac = dot(lightdir.xyz, n);
    
    //if (diffuseFac > 0)
    //{
    //    diffuseColor = float3(DiffuseLight.rgb * DiffuseLight.a * diffuseFac);
    //}
    
    


    //float Fresnel = 0.1f + (1.0f - 0.1f) * pow((1.0f - max(dot(n, -normalize(posEye.xyz)), 0.0f)), 5.0f);
    // Schlickの近似式によるフレネル　  
    
    //float3 L = normalize(-lightdir.xyz - posEye);
    //float3 V = normalize(eyePos.xyz - posEye);
    
    //float3 halfVec = normalize(L + V);
    
            
    //float3 reflection = reflect(-viewDir, n);
    //float3 reflectionTex = cubeMapTexture.Sample(smp, reflection).rgb;
    //float3 reflectionColor = lerp(float3(0.0f, 0.0f, 0.0f), reflectionTex, smoothstep(0.05f, 0.3f, worldPos.y));
    
    // 相対屈折率
    //float ratio = 1.0f / 1.33f;
    
    // 色吸収による？？
    float3 transmission = (1.0f - thickness * 0.2f);
        
    //float3 refraciton = refract(-viewDir, n, ratio);
    float3 refractionColor = FluidColor.rgb * transmission;
            
    float lightattenuration = saturate(dot(normalize(lightDir.xyz), n));
    diffuseColor = diffuseColor.xyz * lightattenuration;

	

	
    //float4 color = FluidColor * diffuseColor;
    //return float4(refractionColor, 1);
    
    // float4 color = float4(lerp(refractionColor, reflectionColor, Fresnel), 1) + float4(specularColor,specularColor,specularColor, 1);
	


    float4 color = float4(lerp(refractionColor, reflectionColor, Fresnel), 1);
    
    //return color;

    return lerp(color, backTexture.Sample(smp, input.uv + n.xy * thickness), 1.0f - thickness);
    
    //return backTexture.Sample(smp, input.uv);
    //return float4(dist_power, dist_power, dist_power, 1.0f);
    //return float4(Transparency, Transparency, Transparency, 1.0f);
    //float4 fluid = float4(diffuse + specular, 1.0f) ;
    //return fluid;
    //return refCol + FluidColor * diffuseColor;
    
    return color;
    //return float4(diffuse, 1);
    //return float4(lightdir, 1);
    //return float4( diffuse, 1.0f);
    //return diffuseColor;

}
