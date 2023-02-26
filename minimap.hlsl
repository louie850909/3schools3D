struct PSINPUT
{
    float4 Position : SV_POSITION;
    float4 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse : COLOR0;
    float4 WorldPos : POSITION0;
    float4 LightPos : POSITION1;
};

struct PSOUTPUT
{
    float4 Diffuse : SV_Target;
};

Texture2D g_TexMiniMap : register(t2);
Texture2D g_TexSSAONormalZMap : register(t3);
Texture2D g_TexSSAORandomMap : register(t4);
Texture2D g_TexSSAOTexMap : register(t5);
Texture2D g_TexSSAOBlurMap : register(t6);
Texture2D g_TexSSAOViewPos : register(t7);
Texture2D g_TexSSAOViewPosBack : register(t8);
SamplerState g_SamplerState : register(s0);

PSOUTPUT MiniMapPS(PSINPUT input)
{
    float4 color;
    PSOUTPUT output;
    
    color = g_TexMiniMap.Sample(g_SamplerState, input.TexCoord);
    color *= input.Diffuse;
    
    output.Diffuse = color;
    
    return output;
}