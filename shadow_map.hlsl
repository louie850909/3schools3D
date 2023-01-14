// マトリクスバッファ
cbuffer WorldBuffer : register(b0)
{
    matrix World;
}

cbuffer ViewBuffer : register(b1)
{
    matrix View;
}

cbuffer ProjectionBuffer : register(b2)
{
    matrix Projection;
}

struct LIGHTMATRIX
{
    float4x4 LightView[5];
    float4x4 LightProjection[5];
};

cbuffer LightMatrixBuffer : register(b8)
{
    LIGHTMATRIX LightMatrix;
}

struct VSINPUT
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct PSINPUT
{
    float4 Position : SV_POSITION;
    //float4 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    //float4 Diffuse : COLOR0;
    //float4 WorldPos : POSITION0;
    //float4 LightPos : POSITION1;
};

struct VS_INSTINPUT
{
    float4 Position : POSITION0;
    float4 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float2 TexCoord : TEXCOORD0;
    float4 inInstancePos : INSTPOS0;
    float4 inInstanceScl : INSTSCL0;
    float4 inInstanceRot : INSTROT0;
};

//--------------------------------------------------------------------
//頂点シェーダ
//--------------------------------------------------------------------
PSINPUT VS(VSINPUT input)
{
    PSINPUT output;
	
    matrix wvp;
	
    wvp = mul(World, LightMatrix.LightView[0]);
    wvp = mul(wvp, LightMatrix.LightProjection[0]);
    output.Position = mul(input.Position, wvp);

    //output.Normal = normalize(mul(float4(input.Normal.xyz, 0.0f), World));

    output.TexCoord = input.TexCoord;

    //output.WorldPos = mul(input.Position, World);

    //output.Diffuse = input.Diffuse;
	
    //output.LightPos = mul(input.Position, World);
    //output.LightPos = mul(output.LightPos, View);
    //output.LightPos = mul(output.LightPos, LightMatrix.LightProjection[0]);
	
    return output;
}

//--------------------------------------------------------------------
//インスタン頂点シェーダ
//--------------------------------------------------------------------
PSINPUT VS_INST(VS_INSTINPUT input)
{
    PSINPUT output;
    
    matrix world;
    matrix rotationX;
    matrix rotationY;
    matrix rotationZ;
    matrix trans;
    matrix scl;
    matrix wvp;
    
    world = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
					 0.0f, 1.0f, 0.0f, 0.0f,
					 0.0f, 0.0f, 1.0f, 0.0f,
					 0.0f, 0.0f, 0.0f, 1.0f);
    
    rotationX = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, cos(input.inInstanceRot.x), sin(input.inInstanceRot.x), 0.0f,
                         0.0f, -sin(input.inInstanceRot.x), cos(input.inInstanceRot.x), 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
	
    rotationY = float4x4(cos(input.inInstanceRot.y), 0.0f, -sin(input.inInstanceRot.y), 0.0f,
										 0.0f, 1.0f, 0.0f, 0.0f,
                         sin(input.inInstanceRot.y), 0.0f, cos(input.inInstanceRot.y), 0.0f,
                                         0.0f, 0.0f, 0.0f, 1.0f);
	
    rotationZ = float4x4(cos(input.inInstanceRot.z), sin(input.inInstanceRot.z), 0.0f, 0.0f,
						 -sin(input.inInstanceRot.z), cos(input.inInstanceRot.z), 0.0f, 0.0f,
                                          0.0f, 0.0f, 1.0f, 0.0f,
                                          0.0f, 0.0f, 0.0f, 1.0f);
	
    trans = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     input.inInstancePos.x, input.inInstancePos.y, input.inInstancePos.z, 1.0f);
	
    scl = float4x4(input.inInstanceScl.x, 0.0f, 0.0f, 0.0f,
                   0.0f, input.inInstanceScl.y, 0.0f, 0.0f,
                   0.0f, 0.0f, input.inInstanceScl.z, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
	
    world = mul(world, scl);
    world = mul(world, rotationX);
    world = mul(world, rotationY);
    world = mul(world, rotationZ);
    world = mul(world, trans);
    
    wvp = mul(world, LightMatrix.LightView[0]);
    wvp = mul(wvp, LightMatrix.LightProjection[0]);
    
    output.Position = mul(input.Position, wvp);
    
    output.TexCoord = input.TexCoord;
    
    return output;
}

PSINPUT VS_GRASS(VS_INSTINPUT input)
{
    PSINPUT output;
    
    matrix wd;
    matrix wvp;
    matrix wlp;
    matrix trans;
    matrix scl;
	
    trans = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     input.inInstancePos.x, input.inInstancePos.y, input.inInstancePos.z, 1.0f);
	
    scl = float4x4(input.inInstanceScl.x, 0.0f, 0.0f, 0.0f,
                   0.0f, input.inInstanceScl.y, 0.0f, 0.0f,
                   0.0f, 0.0f, input.inInstanceScl.z, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
	// ビルドボード
    wd = float4x4(View._11, View._21, View._31, 0.0f,
                  View._12, View._22, View._32, 0.0f,
                  View._13, View._23, View._33, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
    
    wd = mul(wd, scl);
    wd = mul(wd, trans);
    wvp = mul(wd, LightMatrix.LightView[0]);
    wvp = mul(wvp, LightMatrix.LightProjection[0]);
    output.Position = mul(input.Position, wvp);
    output.TexCoord = input.TexCoord;
    
    return output;
}

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);
//--------------------------------------------------------------------
//ピクセルシェーダ
//--------------------------------------------------------------------
 float4 PS(PSINPUT input) : SV_Target
{
    float4 tex = g_Texture.Sample(g_SamplerState, input.TexCoord);
    
    if (tex.a < 0.5f)
    {
        discard;
    }
    
    return float4(input.Position.z, input.Position.z, input.Position.z, 1.0f);
}