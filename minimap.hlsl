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