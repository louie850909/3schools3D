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
}

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
    int fuchi;
    int fill[3];
}


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

struct SSAOCB
{
    matrix ViewToTex;
    float4 FrustumCorners[4];
	
    float OcclusionRadius;
    float OcclusionFadeStart;
    float OcclusionFadeEnd;
    float SurfaceEpsilon;
};

cbuffer SSAOBuffer : register(b11)
{
    SSAOCB SSAO;
}

struct SSAOOV
{
    float4 OffsetVectors[14];
};

cbuffer SSAOOffsetBuffer : register(b12)
{
    SSAOOV OffsetVectors;
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
    float4 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse : COLOR0;
    float4 ViewPos : POSITION0;
    float4 LightPos : POSITION1;
};

struct PSOUTPUT
{
    float4 Diffuse : SV_Target;
};

Texture2D g_TexSSAONormalZMap : register(t3);
Texture2D g_TexSSAORandomMap  : register(t4);
Texture2D g_TexSSAOTexMap     : register(t5);
Texture2D g_TexSSAOBlurMap    : register(t6);
Texture2D g_TexSSAOViewPos    : register(t7);
SamplerState g_SamplerState   : register(s0);

float4x4 inverse(float4x4 m)
{
    float n11 = m[0][0], n12 = m[1][0], n13 = m[2][0], n14 = m[3][0];
    float n21 = m[0][1], n22 = m[1][1], n23 = m[2][1], n24 = m[3][1];
    float n31 = m[0][2], n32 = m[1][2], n33 = m[2][2], n34 = m[3][2];
    float n41 = m[0][3], n42 = m[1][3], n43 = m[2][3], n44 = m[3][3];

    float t11 = n23 * n34 * n42 - n24 * n33 * n42 + n24 * n32 * n43 - n22 * n34 * n43 - n23 * n32 * n44 + n22 * n33 * n44;
    float t12 = n14 * n33 * n42 - n13 * n34 * n42 - n14 * n32 * n43 + n12 * n34 * n43 + n13 * n32 * n44 - n12 * n33 * n44;
    float t13 = n13 * n24 * n42 - n14 * n23 * n42 + n14 * n22 * n43 - n12 * n24 * n43 - n13 * n22 * n44 + n12 * n23 * n44;
    float t14 = n14 * n23 * n32 - n13 * n24 * n32 - n14 * n22 * n33 + n12 * n24 * n33 + n13 * n22 * n34 - n12 * n23 * n34;

    float det = n11 * t11 + n21 * t12 + n31 * t13 + n41 * t14;
    float idet = 1.0f / det;

    float4x4 ret;

    ret[0][0] = t11 * idet;
    ret[0][1] = (n24 * n33 * n41 - n23 * n34 * n41 - n24 * n31 * n43 + n21 * n34 * n43 + n23 * n31 * n44 - n21 * n33 * n44) * idet;
    ret[0][2] = (n22 * n34 * n41 - n24 * n32 * n41 + n24 * n31 * n42 - n21 * n34 * n42 - n22 * n31 * n44 + n21 * n32 * n44) * idet;
    ret[0][3] = (n23 * n32 * n41 - n22 * n33 * n41 - n23 * n31 * n42 + n21 * n33 * n42 + n22 * n31 * n43 - n21 * n32 * n43) * idet;

    ret[1][0] = t12 * idet;
    ret[1][1] = (n13 * n34 * n41 - n14 * n33 * n41 + n14 * n31 * n43 - n11 * n34 * n43 - n13 * n31 * n44 + n11 * n33 * n44) * idet;
    ret[1][2] = (n14 * n32 * n41 - n12 * n34 * n41 - n14 * n31 * n42 + n11 * n34 * n42 + n12 * n31 * n44 - n11 * n32 * n44) * idet;
    ret[1][3] = (n12 * n33 * n41 - n13 * n32 * n41 + n13 * n31 * n42 - n11 * n33 * n42 - n12 * n31 * n43 + n11 * n32 * n43) * idet;

    ret[2][0] = t13 * idet;
    ret[2][1] = (n14 * n23 * n41 - n13 * n24 * n41 - n14 * n21 * n43 + n11 * n24 * n43 + n13 * n21 * n44 - n11 * n23 * n44) * idet;
    ret[2][2] = (n12 * n24 * n41 - n14 * n22 * n41 + n14 * n21 * n42 - n11 * n24 * n42 - n12 * n21 * n44 + n11 * n22 * n44) * idet;
    ret[2][3] = (n13 * n22 * n41 - n12 * n23 * n41 - n13 * n21 * n42 + n11 * n23 * n42 + n12 * n21 * n43 - n11 * n22 * n43) * idet;

    ret[3][0] = t14 * idet;
    ret[3][1] = (n13 * n24 * n31 - n14 * n23 * n31 + n14 * n21 * n33 - n11 * n24 * n33 - n13 * n21 * n34 + n11 * n23 * n34) * idet;
    ret[3][2] = (n14 * n22 * n31 - n12 * n24 * n31 - n14 * n21 * n32 + n11 * n24 * n32 + n12 * n21 * n34 - n11 * n22 * n34) * idet;
    ret[3][3] = (n12 * n23 * n31 - n13 * n22 * n31 + n13 * n21 * n32 - n11 * n23 * n32 - n12 * n21 * n33 + n11 * n22 * n33) * idet;

    return ret;
}

float doAmbientOcclusion(in float2 tcoord, in float2 uv, in float3 p, in float3 cnorm)
{
    float g_scale, g_bias, g_intensity;
    g_scale = 1.0f;
    g_bias = 0.05f;
    g_intensity = 0.35f;
    
    float3 diff = g_TexSSAOViewPos.Sample(g_SamplerState, tcoord + uv).xyz - p;
    const float3 v = normalize(diff);
    const float d = length(diff) * g_scale;
    return max(0.0, dot(cnorm, v) - g_bias) * (1.0 / (1.0 + d)) * g_intensity;
}

Texture2D g_Texture : register(t0);
PSINPUT NormalZMapVS(VSINPUT input)
{
    PSINPUT output;
    float4x4 wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    matrix nWorld = World;
    nWorld[3][0] = 0.0f;
    nWorld[3][1] = 0.0f;
    nWorld[3][2] = 0.0f;
    nWorld[0][3] = 0.0f;
    nWorld[1][3] = 0.0f;
    nWorld[2][3] = 0.0f;
    nWorld[3][3] = 1.0f;
    matrix invTrnWorld = transpose(inverse(nWorld));
    
    matrix wInvTV = mul(invTrnWorld, View);
    
    output.Position = mul(input.Position, wvp);
    output.Normal.xyz = normalize(mul(input.Normal.xyz, (float3x3) wInvTV));
    output.TexCoord = input.TexCoord;
    output.ViewPos = mul(input.Position, World);
    output.ViewPos = mul(output.ViewPos, View);
    
    return output;
}

PSOUTPUT NormalZMapPS(PSINPUT input)
{
    PSOUTPUT output;
    float4 texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    
    float4x4 Wxv = mul(World, View);
    
    float4 Out;
    Out.xyz = input.Normal / 2.0f + 0.5f;
    Out.w = input.ViewPos.z;
    
    output.Diffuse = Out;
    return output;
}

PSINPUT SSAOVS(VSINPUT input)
{
    PSINPUT output;
    
    output.Position = input.Position;
    output.Normal = SSAO.FrustumCorners[input.Normal.x];
    output.TexCoord = input.TexCoord;
    output.ViewPos = input.Position;
    return output;
}

// サンプリング数
#define SPHERE_COUNT 14
PSOUTPUT SSAOPS(PSINPUT input)
{
    PSOUTPUT output;
    
    float g_sample_rad = 0.3f;
    
    const float2 vec[4] = { float2(1, 0), float2(-1, 0), float2(0, 1), float2(0, -1) };
    float3 p = g_TexSSAOViewPos.Sample(g_SamplerState, input.TexCoord).xyz;
    float3 n = g_TexSSAONormalZMap.Sample(g_SamplerState, input.TexCoord).xyz;
    float2 rand = g_TexSSAORandomMap.Sample(g_SamplerState, input.TexCoord).xy;
    rand = rand * 2.0f - 1.0f;
    
    float ao = 0.0f;
    float rad = g_sample_rad / p.z;
    
    int iterations = 4;
    for (int j = 0; j < iterations; ++j)
    {
        float2 coord1 = reflect(vec[j], rand) * rad;
        float2 coord2 = float2(coord1.x * 0.707 - coord1.y * 0.707, coord1.x * 0.707 + coord1.y * 0.707);
        
        ao += doAmbientOcclusion(input.TexCoord, coord1 * 0.25, p, n);
        ao += doAmbientOcclusion(input.TexCoord, coord2 * 0.5, p, n);
        ao += doAmbientOcclusion(input.TexCoord, coord1 * 0.75, p, n);
        ao += doAmbientOcclusion(input.TexCoord, coord2, p, n);
    }
    
    ao /= (float) iterations * 4.0;
    
    float access = 1.0f - ao;
    output.Diffuse.xyz = saturate(pow(access, 4.0f));
    output.Diffuse.w = 1.0f;
    
    return output;
}

// サンプリング数
#define BLUROFFSET_COUNT 24
PSOUTPUT SSAOBlurPS(PSINPUT input)
{
    // ブラーフィルター時のテクセルのオフセット配列
    static float2 BlurOffset24[BLUROFFSET_COUNT] =
    {
        float2(1, 1)
      , float2(-1, 1)
      , float2(-1, -1)
      , float2(1, -1)
      , float2(3, 1)
      , float2(3, 3)
      , float2(1, 3)
      , float2(-1, 3)
      , float2(-3, 3)
      , float2(-3, 1)
      , float2(-3, -1)
      , float2(-3, -3)
      , float2(-1, -3)
      , float2(1, -3)
      , float2(3, -3)
      , float2(3, -1)
        , float2(2, 1)
        , float2(1, 2)
        , float2(2, -1)
        , float2(1,-2)
        , float2(-2, 1)
        , float2(-1,2)
        , float2(-2,-1)
        , float2(-1,-2)
    };
    
    float ScreenWidth = 960.0f;
    float ScreenHeight = 540.0f;
    
    float Out = 0;
    
    for (int i = 0; i < BLUROFFSET_COUNT; i++)
    {
        float2 offset = BlurOffset24[i] / float2(ScreenWidth, ScreenHeight);
        Out += g_TexSSAOTexMap.Sample(g_SamplerState, input.TexCoord + offset).r;
    }
    
    Out /= (float) BLUROFFSET_COUNT;
    
    PSOUTPUT output;
    output.Diffuse = Out;
    return output;
}

struct SSAOVSINSTIN
{
    float4 Position      : POSITION0;
    float4 Normal        : NORMAL0;
    float4 Diffuse       : COLOR0;
    float2 TexCoord      : TEXCOORD0;
    float4 inInstancePos : INSTPOS0;
    float4 inInstanceScl : INSTSCL0;
    float4 inInstanceRot : INSTROT0;
};

struct SSAOPSINSTIN
{
    float4 Position      : SV_POSITION;
    float4 Normal        : NORMAL0;
    float2 TexCoord      : TEXCOORD0;
    float4 Diffuse       : COLOR0;
    float4 ViewPos      : Position0;
    float4 inInstancePos : INSTPOS0;
    float4 inInstanceScl : INSTSCL0;
    float4 inInstanceRot : INSTROT0;
};

struct SSAOPSINSTOUT
{
    float4 Diffuse : SV_Target;
};

SSAOPSINSTIN SSAO_INSTVS(SSAOVSINSTIN input)
{
    SSAOPSINSTIN output;
    matrix world;
    matrix rotationX;
    matrix rotationY;
    matrix rotationZ;
    matrix wvp;
    matrix trans;
    matrix scl;
	
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
    wvp = mul(world, View);
    wvp = mul(wvp, Projection);
    
    matrix nWorld = world;
    nWorld[3][0] = 0.0f;
    nWorld[3][1] = 0.0f;
    nWorld[3][2] = 0.0f;
    nWorld[0][3] = 0.0f;
    nWorld[1][3] = 0.0f;
    nWorld[2][3] = 0.0f;
    nWorld[3][3] = 1.0f;
    matrix invTrnWorld = transpose(inverse(nWorld));
    
    matrix wInvTV = mul(invTrnWorld, View);
    
    output.Position = mul(input.Position, wvp);
    output.Normal.xyz = normalize(mul(input.Normal.xyz, (float3x3)wInvTV));

    output.TexCoord = input.TexCoord;

    output.ViewPos = mul(input.Position, world);
    output.ViewPos = mul(output.ViewPos, View);

    output.Diffuse = input.Diffuse;
    
    output.inInstancePos = input.inInstancePos;
 
    output.inInstanceScl = input.inInstanceScl;
    
    output.inInstanceRot = input.inInstanceRot;
    
    return output;
}

SSAOPSINSTOUT SSAO_INSTPS(SSAOPSINSTIN input)
{
    float4 texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    
    float4 Out;
    Out.xyz = input.Normal / 2.0f + 0.5f;
    Out.w = input.ViewPos.z;
    
    SSAOPSINSTOUT output;
    output.Diffuse = Out;
    
    return output;
}

PSINPUT ViewPosMapVS(VSINPUT input)
{
    PSINPUT output;
    float4x4 wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    output.Position = mul(input.Position, wvp);
    output.Normal = input.Normal;
    output.TexCoord = input.TexCoord;
    output.ViewPos = mul(input.Position, World);
    output.ViewPos = mul(output.ViewPos, View);
    
    return output;
}

PSOUTPUT ViewPosMapPS(PSINPUT input)
{
    PSOUTPUT output;
    float4 texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    
    float4 Out;
    Out = input.ViewPos;
    
    output.Diffuse = Out;
    return output;
}

SSAOPSINSTIN InstViewPosMapVS(SSAOVSINSTIN input)
{
    SSAOPSINSTIN output;
    matrix world;
    matrix rotationX;
    matrix rotationY;
    matrix rotationZ;
    matrix wvp;
    matrix trans;
    matrix scl;
	
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
    wvp = mul(world, View);
    wvp = mul(wvp, Projection);
    
    output.Position = mul(input.Position, wvp);
    output.Normal = input.Normal;

    output.TexCoord = input.TexCoord;

    output.ViewPos = mul(input.Position, world);
    output.ViewPos = mul(output.ViewPos, View);

    output.Diffuse = input.Diffuse;
    
    output.inInstancePos = input.inInstancePos;
 
    output.inInstanceScl = input.inInstanceScl;
    
    output.inInstanceRot = input.inInstanceRot;
    
    return output;
}

SSAOPSINSTOUT InstViewPosMapPS(SSAOPSINSTIN input)
{
    
    float4 texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    
    float4 Out;
    Out = input.ViewPos;
    
    SSAOPSINSTOUT output;
    output.Diffuse = Out;
    
    return output;
}


 

