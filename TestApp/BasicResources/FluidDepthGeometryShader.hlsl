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

static const uint vnum = 4;
static const float size = 0.001f;

static const float4 offset_Cubearray[vnum * 6] =
{
    float4(0.5, -0.5, 0.5, 0),
    float4(0.5, 0.5, 0.5, 0),
    float4(-0.5, -0.5, 0.5, 0),
    float4(-0.5, 0.5, 0.5, 0),
    float4(-0.5, -0.5, -0.5, 0),
    float4(-0.5, 0.5, -0.5, 0),
    float4(0.5, -0.5, -0.5, 0),
    float4(0.5, 0.5, -0.5, 0),
    float4(-0.5, 0.5, -0.5, 0),
    float4(-0.5, 0.5, 0.5, 0),
    float4(0.5, 0.5, -0.5, 0),
    float4(0.5, 0.5, 0.5, 0),
    float4(0.5, -0.5, -0.5, 0),
    float4(0.5, -0.5, 0.5, 0),
    float4(-0.5, -0.5, -0.5, 0),
    float4(-0.5, -0.5, 0.5, 0),
    float4(0.5, -0.5, -0.5, 0),
    float4(0.5, 0.5, -0.5, 0),
    float4(0.5, -0.5, 0.5, 0),
    float4(0.5, 0.5, 0.5, 0),
    float4(-0.5, -0.5, 0.5, 0),
    float4(-0.5, 0.5, 0.5, 0),
    float4(-0.5, -0.5, -0.5, 0),
    float4(-0.5, 0.5, -0.5, 0)
};

static const float4 offset_array[vnum] =
{
    float4(-0.5f, -0.5f, 0, 0),
    float4(-0.5f, +0.5f, 0, 0),
    float4(+0.5f, -0.5f, 0, 0),
    float4(+0.5f, +0.5f, 0, 0),
};

static const float3 uv_Cubearray[6] =
{
    float3(0.0, 0.0, 1.0),
    float3(0.0, 0.0, 1.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(1.0, 0.0, 0.0),
    float3(1.0, 0.0, 0.0)
};

static const float2 uv_array[vnum] =
{
    float2(0,1),
    float2(0,0),
    float2(1,1),
    float2(1,0)
};

float4 ToRoattedPos(float4 pos,float3 rotate)
{
    float degrad = 3.14159265359 / 1.8;
    
    float rx = rotate.x * degrad;
    float ry = rotate.y * degrad;
    float rz = rotate.z * degrad;
    
    float3x3 rotX = {   1, 0,       0, 
                        0, cos(rx), -sin(rx), 
                        0, sin(rx), cos(rx) };
    float3x3 rotY = { cos(ry), 0, sin(ry), 0, 1, 0, -sin(ry), 0, cos(ry) };
    float3x3 rotZ = { cos(rz), -sin(rz), 0, sin(rz), cos(rz), 0, 0, 0, 1 };
    
    
    // ��]�s����|����
    float3 newpos = mul(mul(mul(rotY, rotZ), rotX), pos.xyz);
    
    return float4(newpos * 2.0f,pos.w);
}

[maxvertexcount(vnum * 6)]
void Cubemain(
    point VSOutput input[1] : SV_Position,
    inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
    
    float _scale = input[0].scale * size;
    float4 color = input[0].color;
    
    for (uint j = 0; j < 6; j++)
    {
        for (uint i = 0; i < 4; i++)
        {
            const int vid = j * 4 + i;
            float4 offset = mul(billboard_matrix, offset_Cubearray[vid]);
            element.svpos = input[0].pos + (ToRoattedPos(offset, input[0].rotateAxisAndThita.xyz)) * _scale;
            element.svpos = mul(vp_matrix, element.svpos); // �s��ϊ�
            element.uv = uv_Cubearray[j];
            element.color = color;
            output.Append(element);
        }
        output.RestartStrip();
    }

}


[maxvertexcount(vnum)]
void main(
    point VSOutput input[1] : SV_Position,
    inout TriangleStream<GSOutput> output
)
{
    GSOutput element;
    
    float _scale = input[0].scale * size;
    float4 color = input[0].color;
    
    for (uint i = 0; i < vnum; i++)
    {
        float4 offset = mul(billboard_matrix, offset_array[i]);
        //element.svpos = input[0].pos;
        float3 tempPos = input[0].pos.xyz + offset.xyz * _scale;
    	
        // bilboardMat = VPMat;
        // mat = ViewMat
        element.svpos = mul(vp_matrix, float4(tempPos.xyz, 1.0f)); // �s��ϊ�

    	
        element.scale = (input[0].scale);
    	
        element.pos = mul(view_matrix, float4(tempPos.xyz, 1.0f)).xyz;
        element.uv = uv_array[i];
        element.color = color;

        output.Append(element);
    }
    
}