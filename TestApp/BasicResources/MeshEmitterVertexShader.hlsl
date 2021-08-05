cbuffer cbuff0 : register(b0)
{
    matrix mat;
    matrix billboardMat;
};

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



struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    //float4 scale : TEXCOORD2;
};

matrix GetRotateMatrix(float3 rotate)
{
    float degrad = 3.14159265359 / 1.8;
    
    float rx = rotate.x * degrad;
    float ry = rotate.y * degrad;
    float rz = rotate.z * degrad;
    
    matrix rotX =
    {
        1, 0, 0,0,
        0, cos(rx), -sin(rx),0,
        0, sin(rx), cos(rx),0,
        0,0,0,1
    };
    
    matrix rotY =
    {
        cos(ry), 0, sin(ry), 0,
        0, 1, 0, 0,
        sin(ry), 0, cos(ry),0,
        0, 0, 0, 1
    };
    
    matrix rotZ = 
    {
        cos(rz), -sin(rz), 0, 0,
        sin(rz), cos(rz), 0, 0,
        0, 0, 1,0,
        0, 0, 0,1
    };
    
    
    // 回転行列を掛ける
    return mul(mul(rotY, rotZ), rotX);
    
}

StructuredBuffer<PosVelo> g_bufPosVelo;
VSOutput main(uint id : SV_InstanceID, float4 pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
    VSOutput output;
    
    matrix worldMat;
    
    float3 particleRotate = g_bufPosVelo[id].rotateAxis;
    float3 particlePos = g_bufPosVelo[id].pos;

    
    // パーティクルデータの位置情報を元に行列を生成
    matrix transMat = matrix
    (1.0f, 0, 0, 0,
    0, 1.0f, 0, 0,
    0, 0, 1.0f, 0,
    particlePos.x, particlePos.y, particlePos.z, 1);
    
    matrix scaleMat = matrix
    (g_bufPosVelo[id].scale.x, 0, 0, 0,
    0, g_bufPosVelo[id].scale.x, 0, 0,
    0, 0, g_bufPosVelo[id].scale.x, 0,
    0,0,0, 1);
    

    
    scaleMat = transpose(scaleMat);
    transMat = transpose(transMat);
    matrix rotateMat = transpose(GetRotateMatrix(particleRotate));
    
    //worldMat = mul(transMat,scaleMat);
    //worldMat = translate(float3(0, 0, 30));
    //worldMat = billboardMat;
    //worldMat = mul(transMat, scaleMat);
    worldMat = mul(mul(transMat, rotateMat), scaleMat);
    
    matrix wvpMat = mul(mat, worldMat);
    
    // パーティクルデータの位置を元に座標変換
    output.pos = mul(wvpMat,pos);
    output.uv = uv;
    //output.pos = mul(output.pos, mat);
    //output.axisAndThita = g_bufPosVelo[input.id].rotateAxis;
    //output.scale = g_bufPosVelo[input.id].scale;
    output.color = g_bufPosVelo[id].color;

    return output;
}