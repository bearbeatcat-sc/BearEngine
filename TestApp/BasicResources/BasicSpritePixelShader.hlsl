cbuffer cbuff0 : register(b0)
{
    float4 color;
    matrix mat; // 3D�ϊ��s��
}

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0); // 0�ŃX���b�g�̃e�N�X�`��
SamplerState smp : register(s0); // 0�ŃX���b�g�ɐݒ肳�ꂽ�T���v���[

float4 main(VSOutput input) : SV_TARGET
{
    return float4(tex.Sample(smp,input.uv)) * color;
    // return float4(1, 1, 1, 1);

}