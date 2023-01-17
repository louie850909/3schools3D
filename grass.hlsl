
//*****************************************************************************
// 定数バッファ
//*****************************************************************************

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

// マテリアルバッファ
struct MATERIAL
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emission;
    float Shininess;
    int noTexSampling;
    float Dummy[2]; //16byte境界用
};

cbuffer MaterialBuffer : register(b3)
{
    MATERIAL Material;
}

// ライト用バッファ
struct LIGHT
{
    float4 Direction[5];
    float4 Position[5];
    float4 Diffuse[5];
    float4 Ambient[5];
    float4 Attenuation[5];
    int4 Flags[5];
    int Enable;
    int Dummy[3]; //16byte境界用
};

cbuffer LightBuffer : register(b4)
{
    LIGHT Light;
}

struct FOG
{
    float4 Distance;
    float4 FogColor;
    int Enable;
    float Dummy[3]; //16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register(b5)
{
    FOG Fog;
};

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
    int fuchi;
    int fill[3];
};


cbuffer CameraBuffer : register(b7)
{
    float4 Camera;
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

cbuffer TimeBuffer : register(b9)
{
    float4 Time;
}

//=============================================================================
// インスタンシンVSシェーダ
//=============================================================================
void VertexShaderInstancing(in float4 inPosition : POSITION0,
							in float4 inNormal : NORMAL0,
							in float4 inDiffuse : COLOR0,
							in float2 inTexCoord : TEXCOORD0,
							in float4 inInstancePos : INSTPOS,
							in float4 inInstanceScl : INSTSCL,
                            in float4 inInstanceRot : INSTROT,

							out float4 outPosition : SV_POSITION,
							out float4 outNormal : NORMAL0,
							out float2 outTexCoord : TEXCOORD0,
							out float4 outDiffuse : COLOR0,
							out float4 outWorldPos : POSITION0,
                            out float4 LightPos : POSITION1)
{
    matrix wd;
    matrix wvp;
    matrix wlp;
    matrix trans;
    matrix scl;
	
    trans = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
                     0.0f, 1.0f, 0.0f, 0.0f,
                     0.0f, 0.0f, 1.0f, 0.0f,
                     inInstancePos.x, inInstancePos.y, inInstancePos.z, 1.0f);
	
    scl = float4x4(inInstanceScl.x, 0.0f, 0.0f, 0.0f,
                   0.0f, inInstanceScl.y, 0.0f, 0.0f,
                   0.0f, 0.0f, inInstanceScl.z, 0.0f,
                   0.0f, 0.0f, 0.0f, 1.0f);
	// ビルドボード
    wd = float4x4(View._11, View._21, View._31, 0.0f,
                  View._12, View._22, View._32, 0.0f,
                  View._13, View._23, View._33, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
    
    wd = mul(wd, scl);
    wd = mul(wd, trans);
    wvp = mul(wd, View);
    wvp = mul(wvp, Projection);
    // xz bias for grass wave
    float x = sin(Time.x + inInstancePos.x) * 2.0f;
    float z = sin(Time.x + inInstancePos.x) * 2.0f;
    
    if (inTexCoord.y == 0.0f)
    {
        outPosition = mul(float4(inPosition.x + x, inPosition.y, inPosition.z + z, 1.0f), wvp);
    }
    else
    {
        outPosition = mul(inPosition, wvp);
    }
	
    outNormal = normalize(mul(float4(inNormal.xyz, 0.0f), wd));

    outTexCoord = inTexCoord;
	
    outWorldPos = mul(inInstancePos, wd);

    outDiffuse = inDiffuse;
    
    wlp = mul(wd, LightMatrix.LightView[0]);
    wlp = mul(wlp, LightMatrix.LightProjection[0]);
    LightPos = mul(inPosition, wlp);
}