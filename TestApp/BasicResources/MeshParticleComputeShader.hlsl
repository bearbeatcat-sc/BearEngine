
#define blocksize 256

struct PosVelo
{
    float4 pos;
    float4 velo;
    float4 rotateAxis;
    float4 color;
    float4 scale;
    bool activeFlag;
    float generateTime;
    float lifeTime;
};

cbuffer ParticleUpdateParams : register(b0)
{
    float4 addVeclotiy;
    float4 addRotate;
    float4 addColor;
    float4 addScale;    
    
    float4 ColorRange;
    float4 PositionRange;
    float4 VecRange;
    float4 ScaleRange;
    float4 RotateRange;        
    float LifeTimeRange;       
};

cbuffer EmiiterDataParams : register(b1)
{
    float4 BasePosition;
    float4 BaseColor;
    float4 BaseVelocity;
    float4 BaseScale;
    float4 BaseRotate;
    float RandomSeed;
    float BaseLifeTime;
    float TimeScale;
    int ParticleCount;
}

StructuredBuffer<PosVelo> oldPosVelo : register(t0); // SRV
StructuredBuffer<float3> verticesPositions : register(t1); // SRV
RWStructuredBuffer<PosVelo> newPosVelo : register(u0); // UAV


inline float rand(float2 seed)
{
    return frac(sin(dot(seed.xy, float2(12.9898, 78.233))) * 43758.5453);
}

inline float3 rand3(float2 seed)
{
    return 2.0 * (float3(rand(seed * 1), rand(seed * 2), rand(seed * 3)) - 0.5);
}

inline float xorShiftRand(int seed)
{
    seed ^= (seed << 24);
    seed ^= (seed >> 1);
    seed *= 0x456789abu;
    seed ^= (seed << 1);
    seed *= 0x456789abu;
    
    return float(seed) / float(0xffffffff);

}

inline float3 xorShiftRand3(int3 seed)
{
    seed ^= (seed.yzx << 24);
    seed ^= (seed.yzx >> 1);
    seed *= 0x456789abu;
    seed ^= (seed.yzx << 1);
    seed *= 0x456789abu;
    
    return float3(seed) / float(0xffffffff);
}

[numthreads(blocksize, 1, 1)]
void init(uint3 DTid : SV_DispatchThreadID)
{
    if (newPosVelo[DTid.x].activeFlag == true)
    {
        return;
    }
    
    float3 seed = float3(RandomSeed * DTid.x, RandomSeed * DTid.x + 1, RandomSeed * DTid.x + 2);
    //float3 pos = xorShiftRand3(seed);
    float3 vel = xorShiftRand3(seed);
    float3 rotate = xorShiftRand3(seed);
    float3 scale = xorShiftRand3(seed);
    float3 color = xorShiftRand3(seed);
        
    PosVelo posVelo = newPosVelo[DTid.x];
    posVelo.color = BaseColor + float4(ColorRange.xyz * color, 1.0f);
    
    // ƒ‚ƒfƒ‹‚Ì’¸“_‚Å\¬‚·‚é‚Ì‚Å
    posVelo.pos = BasePosition + float4(verticesPositions[DTid.x].xyz, 1.0f);
    
    posVelo.rotateAxis = BaseRotate + float4(RotateRange.xyz * rotate, 1.0f);
    posVelo.scale = BaseScale + float4(ScaleRange.xyz * scale, 1.0f);
    posVelo.velo = BaseVelocity + float4(VecRange.xyz * vel, 1.0f);
    posVelo.generateTime = 0.0f;
    posVelo.lifeTime = BaseLifeTime + LifeTimeRange * xorShiftRand(seed.x);
    posVelo.activeFlag = true;
 
    newPosVelo[DTid.x] = posVelo;
}

[numthreads(blocksize, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float4 pos = oldPosVelo[DTid.x].pos;
    float4 vel = oldPosVelo[DTid.x].velo;
    float4 rotateAxis = oldPosVelo[DTid.x].rotateAxis;
    float4 scale = oldPosVelo[DTid.x].scale;
    float4 color = oldPosVelo[DTid.x].color;
    bool activeFlag = oldPosVelo[DTid.x].activeFlag;
    float generateTime = oldPosVelo[DTid.x].generateTime;
    float lifeTime = oldPosVelo[DTid.x].lifeTime;
        
    if (generateTime >= lifeTime)
    {
        newPosVelo[DTid.x].activeFlag = false;
        return;
    }
    
    pos += float4(vel.x, vel.y, vel.z, 0.0f) * TimeScale;
    scale += addScale * TimeScale;
    rotateAxis += addRotate * TimeScale;
    vel += addVeclotiy * TimeScale;
    color += addColor * TimeScale;
    generateTime += 0.01f * TimeScale;
        
    newPosVelo[DTid.x].pos = pos;
    newPosVelo[DTid.x].velo = vel;
    newPosVelo[DTid.x].rotateAxis = rotateAxis;
    newPosVelo[DTid.x].scale = scale;
    newPosVelo[DTid.x].color = color;
    newPosVelo[DTid.x].activeFlag = activeFlag;
    newPosVelo[DTid.x].generateTime = generateTime;
    newPosVelo[DTid.x].lifeTime = lifeTime;
}