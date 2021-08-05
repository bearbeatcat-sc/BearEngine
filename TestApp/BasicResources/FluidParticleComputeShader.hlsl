#define blocksize 256

//1粒子分のデータ
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

//密度
struct Density
{
	float Density;
    float offset0;
    float offset1;
    float offset2;
};

//圧力
struct Forces
{
	float4 accleration;
	float pressure;
    float offset0;
    float offset1;
    float offset2;
};

//壁粒子用データ
struct WallPalam
{
	float4 position;
    float4 scale;
	float pressure;
};

//流体計算用データ
cbuffer ParticleUpdateParams : register(b0)
{
	float4 gravity;
	float timeStep;
	float maxVelocity;
	float radiusForNumberDensity;
	float radiusForGradient;
	float radiusForLaplacian;
	float n0ForNumberDensity;
	float n0ForGradient;
	float n0ForLaplacian;
	float lambda;
	int DIM;
	float PARTICLE_DISTANCE;
	float DT;
	float COEFFICIENT_OF_RESTITUTION;
	float KINEMATIC_VISCOSITY;
	float FLUID_DENSITY;
	float wallCount;
};

//エミッター用データ
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

StructuredBuffer<Density> oldDensity : register(t2); // SRV
RWStructuredBuffer<Density> newDensity : register(u1); // UAV

StructuredBuffer<Forces> oldForces : register(t3); // SRV
RWStructuredBuffer<Forces> newForces : register(u2); // UAV

StructuredBuffer<WallPalam> oldWallPalam : register(t4); // SRV
RWStructuredBuffer<WallPalam> newWallPalam : register(u3); // UAV

//疑似ランダム(float)
inline float xorShiftRand(int seed)
{
	seed ^= (seed << 24);
	seed ^= (seed >> 1);
	seed *= 0x456789abu;
	seed ^= (seed << 1);
	seed *= 0x456789abu;

	return float(seed) / float(0xffffffff);
}

//疑似ランダム(float3)
inline float3 xorShiftRand3(int3 seed)
{
	seed ^= (seed.yzx << 24);
	seed ^= (seed.yzx >> 1);
	seed *= 0x456789abu;
	seed ^= (seed.yzx << 1);
	seed *= 0x456789abu;

	return float3(seed) / float(0xffffffff);
}

//距離関数
float Distance2(float3 v1, float3 v2)
{
	float3 v = v2 - v1;
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

//流体計算用重み関数
float Weight(float dist, float re)
{
	return dist >= re ? 0.0f : (re / dist) - 1.0f;
}

//重力計算
//void CalcGravity(uint3 DTid)
//{
//	unsigned int P_ID = DTid.x;
//	float3 accelaration = float3(0, 0, 0);

//	accelaration = float3(gravity.xyz);

//	newForces[P_ID].accleration = accelaration;
//}

groupshared
struct
{
    float4 wallPosition;
    float4 velocity;
    float4 position;
    float pressure;
    float wallPressure;
    float padding0;
    float padding1;
} shared_Data[blocksize];

//密度計算
void CalcViscosity(
	uint3 Gid,
	uint3 DTid,
	uint3 GTid,
	uint GI)
{
	float a = (KINEMATIC_VISCOSITY) * (2.0f * DIM) / (n0ForLaplacian * lambda);

	unsigned int P_ID = DTid.x;

	float3 P_position = float3(oldPosVelo[P_ID].pos.xyz);
	float3 P_velocity = float3(oldPosVelo[P_ID].velo.xyz);
    float3 accleration = oldForces[P_ID].accleration.xyz;

	accleration = gravity.xyz;
	float3 viscosityTerm = float3(0, 0, 0);

	//流体粒子同士の計算
	[loop]
	for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
	{
        shared_Data[GI].position = float4(oldPosVelo[N_block_ID + GI].pos.xyz,1);
        shared_Data[GI].velocity = float4(oldPosVelo[N_block_ID + GI].velo.xyz,1);

		GroupMemoryBarrierWithGroupSync();

		[loop]
		for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
		{
            uint N_ID = N_block_ID + N_tile_ID;
            float3 N_position = shared_Data[N_tile_ID].position.xyz;
            float3 N_velocity = shared_Data[N_tile_ID].velocity.xyz;

            float dist2 = Distance2(P_position, N_position);

            if (dist2 < 0.00001f)
            {
                dist2 = 0.00001f;
            }

            float dist = sqrt(dist2);

            if (dist < radiusForLaplacian && N_ID != P_ID)
            {
                float w = Weight(dist, radiusForLaplacian);
                viscosityTerm += (N_velocity - P_velocity) * w;
            }
        }
		
	}

	GroupMemoryBarrierWithGroupSync();

	// 壁粒子との判定
	[loop]
	for (uint N_block_ID = 0; N_block_ID < (uint) wallCount; N_block_ID += blocksize)
	{
        shared_Data[GI].wallPosition = float4(float3(oldWallPalam[N_block_ID + GI].position.xyz), 1);

		GroupMemoryBarrierWithGroupSync();

		[loop]
		for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
		{
            uint N_ID = N_block_ID + N_tile_ID;
            float3 N_position = shared_Data[N_tile_ID].wallPosition.xyz;

            float dist2 = Distance2(P_position, N_position);

            if (dist2 < 0.00001f)
            {
                dist2 = 0.00001f;
            }

            float dist = sqrt(dist2);

            if (dist < radiusForLaplacian && N_ID != P_ID)
            {
                float w = Weight(dist, radiusForLaplacian);
                viscosityTerm += (float3(0, 0, 0) - P_velocity) * w;
            }
        }
		
	}

	GroupMemoryBarrierWithGroupSync();

	accleration += viscosityTerm * a;

    newForces[P_ID].accleration = float4(accleration, 0);
}

//粒子の仮移動
void MoveParticle(
	uint3 DTid)
{
	const unsigned int P_ID = DTid.x;

	float3 position = float3(oldPosVelo[P_ID].pos.xyz);
	float3 velocity = float3(oldPosVelo[P_ID].velo.xyz);
	float3 accleration = float3(oldForces[P_ID].accleration.xyz);

	velocity += accleration * timeStep;
	position += velocity * timeStep;

	newPosVelo[P_ID].pos = float4(position, 1.0f);
	newPosVelo[P_ID].velo = float4(velocity, 1.0f);

    newForces[P_ID].accleration = float4(0.0f, 0.0f, 0.0f, 0.0f);
}

//当たり判定(例外処理)
void Collision(
	uint3 Gid,
	uint3 DTid,
	uint3 GTid,
	uint GI)
{
	float e = COEFFICIENT_OF_RESTITUTION;

	const unsigned int P_ID = DTid;

	float3 P_position = float3(oldPosVelo[P_ID].pos.xyz);
	float3 P_velocity = float3(oldPosVelo[P_ID].velo.xyz);

	float3 VelocityAfterCollision = P_velocity;

	float mi = FLUID_DENSITY;

	float collisionDistance = (1.0f * PARTICLE_DISTANCE);
	float collisionDistance2 = collisionDistance * collisionDistance;

	[loop]
	for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
	{
        shared_Data[GI].position = float4(float3(oldPosVelo[N_block_ID + GI].pos.xyz), 1);
        shared_Data[GI].velocity = float4(float3(oldPosVelo[N_block_ID + GI].velo.xyz), 1);

		GroupMemoryBarrierWithGroupSync();

		[loop]
		for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
		{
            uint N_ID = N_block_ID + N_tile_ID;
            float3 N_position = shared_Data[N_tile_ID].position.xyz;
            float3 N_velocity = shared_Data[N_tile_ID].velocity.xyz;

            float3 ij = N_position - P_position;
            float dist2 = Distance2(P_position, N_position);

            if (dist2 < 0.00001f)
            {
                dist2 = 0.00001f;
            }

            if (dist2 < collisionDistance2 && P_ID != N_ID)
            {
                float dist = sqrt(dist2);

                float forceDT = (VelocityAfterCollision.x - N_velocity.x) * (ij.x / dist)
					+ (VelocityAfterCollision.y - N_velocity.y) * (ij.y / dist)
					+ (VelocityAfterCollision.z - N_velocity.z) * (ij.z / dist);

                if (forceDT > 0.0f)
                {
                    float mj = FLUID_DENSITY;
                    forceDT *= (1.0f * e) * mi * mj / (mi + mj);
                    VelocityAfterCollision -= (forceDT / mi) * (ij / dist);
                }
            }
        }
		
        GroupMemoryBarrierWithGroupSync();
	}

	GroupMemoryBarrierWithGroupSync();

	P_position += (VelocityAfterCollision - P_velocity) * timeStep;
	P_velocity = VelocityAfterCollision;

	newPosVelo[P_ID].pos = float4(P_position, 1.0f);
	newPosVelo[P_ID].velo = float4(P_velocity, 1.0f);
}

//圧力計算
void CalcPressure(
	uint3 Gid,
	uint3 DTid,
	uint3 GTid,
	uint GI)
{
	unsigned int P_ID = DTid.x;
	float3 P_position = float3(oldPosVelo[P_ID].pos.xyz);
	float3 P_velocity = float3(oldPosVelo[P_ID].velo.xyz);

	float radiusForNumberDensity2 = radiusForNumberDensity * radiusForNumberDensity;
	float ni = 0.0f;

	[loop]
	for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
	{
        shared_Data[GI].position = float4(float3(oldPosVelo[N_block_ID + GI].pos.xyz), 1);
		GroupMemoryBarrierWithGroupSync();

		[loop]
		for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
		{
			uint N_ID = N_block_ID + N_tile_ID;
            float3 N_position = shared_Data[N_tile_ID].position.xyz;

            float dist2 = Distance2(P_position, N_position);

            if (dist2 < 0.00001f)
            {
                dist2 = 0.00001f;
            }

            if (dist2 < radiusForNumberDensity2 && N_ID != P_ID)
            {
                float dist = sqrt(dist2);
                float w = Weight(dist, radiusForNumberDensity);
                ni += w;
            }
        }
		
        GroupMemoryBarrierWithGroupSync();
	}

	GroupMemoryBarrierWithGroupSync();

	[loop]
	for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
	{
        shared_Data[GI].wallPosition = float4(float3(oldWallPalam[N_block_ID + GI].position.xyz), 1);
		GroupMemoryBarrierWithGroupSync();

		[loop]
		for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
		{
			uint N_ID = N_block_ID + N_tile_ID;
            float3 N_position = shared_Data[N_tile_ID].wallPosition.xyz;

            float dist2 = Distance2(P_position, N_position);

            if (dist2 < 0.00001f)
            {
                dist2 = 0.00001f;
            }

            if (dist2 < radiusForNumberDensity2 && N_ID != P_ID)
            {
                float dist = sqrt(dist2);
                float w = Weight(dist, radiusForNumberDensity);
                ni += w;
            }
        }
		
        GroupMemoryBarrierWithGroupSync();
	}
	
	GroupMemoryBarrierWithGroupSync();

	float pressure = (ni > n0ForNumberDensity) * (ni - n0ForNumberDensity) * 22.0f * 22.0f / n0ForNumberDensity * 1000;
	
    pressure = 0.3f;
	newForces[P_ID].pressure = pressure;
}


//圧力勾配計算
void CalcPressureGradient(
	uint3 Gid,
	uint3 DTid,
	uint3 GTid,
	uint GI)
{
	unsigned int P_ID = DTid.x;

	float3 P_position = float3(oldPosVelo[P_ID].pos.xyz);
	float P_pressure = oldForces[P_ID].pressure;
	float3 acc = float3(0, 0, 0);

	float radiusForNumberDensity2 = radiusForNumberDensity * radiusForNumberDensity;

	float pre_min = P_pressure;

	//流体粒子同士の計算
	{
		[loop]
		for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
		{
            shared_Data[GI].position = float4(float3(oldPosVelo[N_block_ID + GI].pos.xyz), 1);
            shared_Data[GI].pressure = oldForces[N_block_ID + GI].pressure;

			GroupMemoryBarrierWithGroupSync();

			[loop]
			for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
			{
				uint N_ID = N_block_ID + N_tile_ID;
                float3 N_position = shared_Data[N_tile_ID].position.xyz;
                float N_pressure = shared_Data[N_tile_ID].pressure;

                float dist2 = Distance2(P_position, N_position);

                if (dist2 < 0.00001f)
                {
                    dist2 = 0.00001f;
                }

                if (dist2 < radiusForNumberDensity2 && P_ID != N_ID)
                {
                    if (pre_min > N_pressure)
                    {
                        pre_min = N_pressure;
                    }
                }
            }
			
            GroupMemoryBarrierWithGroupSync();
		}

		GroupMemoryBarrierWithGroupSync();
		
		[loop]
		for (uint N_block_ID = 0; N_block_ID < (uint) ParticleCount; N_block_ID += blocksize)
		{
            shared_Data[GI].position = float4(float3(oldPosVelo[N_block_ID + GI].pos.xyz), 1);
            shared_Data[GI].pressure = oldForces[N_block_ID + GI].pressure;

			GroupMemoryBarrierWithGroupSync();

			[loop]
			for (uint N_tile_ID2 = 0; N_tile_ID2 < blocksize; N_tile_ID2++)
			{
				uint N_ID = N_block_ID + N_tile_ID2;
                float3 N_position = shared_Data[N_tile_ID2].position.xyz;
                float N_pressure = shared_Data[N_tile_ID2].pressure;
				
                float dist2 = Distance2(P_position, N_position);

                if (dist2 < 0.00001f)
                {
                    dist2 = 0.00001f;
                }

                if (dist2 < radiusForNumberDensity2 && P_ID != N_ID)
                {
                    float dist = sqrt(dist2);
                    float w = Weight(dist, radiusForNumberDensity);
					
                    w *= (N_pressure - pre_min) / dist2;
					
                    float3 v = N_position - P_position;
                    acc += v * w;
                }
            }
			
            GroupMemoryBarrierWithGroupSync();
		}

	}

	GroupMemoryBarrierWithGroupSync();

    // 壁粒子との計算
	{
		[loop]
        for (uint N_block_ID = 0; N_block_ID < (uint) wallCount; N_block_ID += blocksize)
        {
            shared_Data[GI].wallPosition = float4(float3(oldWallPalam[N_block_ID + GI].position.xyz), 1);
            shared_Data[GI].wallPressure = oldWallPalam[N_block_ID + GI].pressure;

            GroupMemoryBarrierWithGroupSync();

			[loop]
            for (uint N_tile_ID = 0; N_tile_ID < blocksize; N_tile_ID++)
            {
                uint N_ID = N_block_ID + N_tile_ID;
                float3 N_position = shared_Data[N_tile_ID].wallPosition.xyz;
                float N_pressure = shared_Data[N_tile_ID].wallPressure;

                float dist2 = Distance2(P_position, N_position);

                if (dist2 < 0.00001f)
                {
                    dist2 = 0.00001f;
                }

                if (dist2 < radiusForNumberDensity2 && P_ID != N_ID)
                {
                    if (pre_min > N_pressure)
                    {
                        pre_min = N_pressure;
                    }
                }
            }
			
            GroupMemoryBarrierWithGroupSync();
        }

        GroupMemoryBarrierWithGroupSync();

		[loop]
        for (uint N_block_ID = 0; N_block_ID < (uint) wallCount; N_block_ID += blocksize)
        {
            shared_Data[GI].wallPosition = float4(float3(oldWallPalam[N_block_ID + GI].position.xyz), 1);
            shared_Data[GI].wallPressure = oldWallPalam[N_block_ID + GI].pressure;

            GroupMemoryBarrierWithGroupSync();

			[loop]
            for (uint N_tile_ID2 = 0; N_tile_ID2 < blocksize; N_tile_ID2++)
            {
                uint N_ID = N_block_ID + N_tile_ID2;
                float3 N_position = shared_Data[N_tile_ID2].wallPosition.xyz;
                float N_pressure = shared_Data[N_tile_ID2].wallPressure;

                float dist2 = Distance2(P_position, N_position);

                if (dist2 < 0.00001f)
                {
                    dist2 = 0.00001f;
                }

                if (dist2 < radiusForNumberDensity2 && P_ID != N_ID)
                {
                    float dist = sqrt(dist2);
                    float w = Weight(dist, radiusForNumberDensity);
                	
                    w *= (N_pressure - pre_min) / dist2;
                    float3 v = N_position - P_position;
                    acc += v * w;
                }
            }
			
            GroupMemoryBarrierWithGroupSync();
        }
    }

	GroupMemoryBarrierWithGroupSync();

    newForces[P_ID].accleration = float4((acc * (1.0f / 1000.0f) * -DIM / n0ForNumberDensity), 1);
}

//粒子の本移動
void MoveParticleUsingPressureGradient(
	uint3 DTid : SV_DispatchThreadID)
{
	unsigned int P_ID = DTid.x;

	float3 position = float3(oldPosVelo[P_ID].pos.xyz);
	float3 velocity = float3(oldPosVelo[P_ID].velo.xyz);
	float3 acceration = float3(oldForces[P_ID].accleration.xyz);

    velocity += gravity.xyz * timeStep;
	velocity += acceration * timeStep;
	position += velocity * timeStep;

	newPosVelo[P_ID].pos = float4(position, 1.0f);
	newPosVelo[P_ID].velo = float4(velocity, 1.0f);
}

[numthreads(blocksize, 1, 1)]
void init(
	uint3 DTid : SV_DispatchThreadID)
{
	if (DTid.x > ParticleCount)
	{
		return;
	}

	if (newPosVelo[DTid.x].activeFlag == true)
	{
		return;
	}

	float3 seed = float3(RandomSeed * DTid.x, RandomSeed * DTid.x + 1, RandomSeed * DTid.x + 2);
	float3 pos = xorShiftRand3(seed);
	float3 vel = xorShiftRand3(seed);
	float3 rotate = xorShiftRand3(seed);
	float3 scale = xorShiftRand3(seed);
	float3 color = xorShiftRand3(seed);

	PosVelo posVelo = newPosVelo[DTid.x];
	posVelo.color = BaseColor;

	int x = DTid.x % 10 - 5;
	int y = DTid.x / 10 % 10 - 5;
	int z = DTid.x / 100 % 10 - 5;

	posVelo.pos = BasePosition + float4(float3(x, y, z) * (PARTICLE_DISTANCE * 0.1f), 0);
	posVelo.rotateAxis = BaseRotate;
	posVelo.scale = BaseScale;
	//posVelo.velo = float4(0.0f, 0.0f, 0.0f, 0.0f);
	posVelo.velo = float4(/*float3(200, 200, 200) * */vel, 1.0f);
	posVelo.generateTime = 0.0f;
	posVelo.lifeTime = BaseLifeTime;
	posVelo.activeFlag = true;

	newPosVelo[DTid.x] = posVelo;


	//WallPalam wallPalam;
	//int xx = DTid.x % 20 - 10;
	//int yy = DTid.x / 10 % 10 - 5;
	//int zz = DTid.x / 100 % 20 - 10;
 //   wallPalam.position = float4(float3(xx, yy, zz) * (PARTICLE_DISTANCE * 0.1f), 1.0f);
 //   wallPalam.scale = PARTICLE_DISTANCE;
 //   wallPalam.pressure = 0.0f;

 //   newWallPalam[DTid.x] = wallPalam;
}

[numthreads(blocksize, 1, 1)]
void CS_Force(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	//CalcGravity(DTid);
	CalcViscosity(Gid, DTid, GTid, GI);
}

[numthreads(blocksize, 1, 1)]
void CS_MoveParticle(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	MoveParticle(DTid);
}

[numthreads(blocksize, 1, 1)]
void CS_Collision(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	//Collision(Gid, DTid, GTid, GI);
}

[numthreads(blocksize, 1, 1)]
void CS_Pressure(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	CalcPressure(Gid, DTid, GTid, GI);
}

[numthreads(blocksize, 1, 1)]
void CS_PressureGradient(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	CalcPressureGradient(Gid, DTid, GTid, GI);
}

[numthreads(blocksize, 1, 1)]
void CS_Integrate(
	uint3 Gid : SV_GroupID,
	uint3 DTid : SV_DispatchThreadID,
	uint3 GTid : SV_GroupThreadID,
	uint GI : SV_GroupIndex)
{
	MoveParticleUsingPressureGradient(DTid);
}