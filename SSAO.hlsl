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
    float4 OffsetVectors[14];
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

float OcculusionFunction(float distZ)
{
    float occ = 0.0f;
    if(distZ > SSAO.SurfaceEpsilon)
    {
        float fadeLength = SSAO.OcclusionFadeEnd - SSAO.OcclusionFadeStart;
        occ = saturate((SSAO.OcclusionFadeEnd - distZ) / fadeLength);
    }
    return occ;
}

float4 getViewPos(float2 texCoord, float z)
{
    float4 worldPos;
    
    // まず正規化スクリーン座標系での座標を計算する。
    // z座標は深度テクスチャから引っ張ってくる。
    worldPos.z = z;
    // xy座標はUV座標から計算する。
    worldPos.xy = texCoord * float2(2.0f, -2.0f) - 1.0f;
    worldPos.w = 1.0f;
    // ビュープロジェクション行列の逆行列を乗算して、ワールド座標に戻す。
    worldPos = mul(inverse(Projection), worldPos);
    worldPos.xyz /= worldPos.w;
    
    float4 ViewPos;
    ViewPos = mul(worldPos, View);
    return ViewPos;
}

Texture2D g_Texture : register(t0);
PSINPUT NormalZMapVS(VSINPUT input)
{
    PSINPUT output;
    float4x4 wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    matrix invTrnWorld = transpose(inverse(World));
    invTrnWorld[3][0] = 0.0f;
    invTrnWorld[3][1] = 0.0f;
    invTrnWorld[3][2] = 0.0f;
    invTrnWorld[0][3] = 0.0f;
    invTrnWorld[1][3] = 0.0f;
    invTrnWorld[2][3] = 0.0f;
    invTrnWorld[3][3] = 1.0f;
    
    matrix invTrnView = transpose(inverse(View));
    invTrnView[3][0] = 0.0f;
    invTrnView[3][1] = 0.0f;
    invTrnView[3][2] = 0.0f;
    invTrnView[0][3] = 0.0f;
    invTrnView[1][3] = 0.0f;
    invTrnView[2][3] = 0.0f;
    invTrnView[3][3] = 1.0f;
    
    float3x3 wv = mul((float3x3)World, (float3x3)View);
    float4x4 wv4 = float4x4(
        float4(wv[0],   0),
        float4(wv[1],   0),
        float4(wv[2],   0),
        float4(0, 0, 0, 1)
    );
    
    output.Position = mul(input.Position, wvp);
    output.Normal = mul(input.Normal, invTrnWorld);
    output.Normal = normalize(mul(output.Normal, invTrnView));
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
    //Out.xyz = input.Normal.xyz;
    Out.xyz = (input.Normal.xyz / 2.0f) + 0.5f;
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
    
    float3 normal = g_TexSSAONormalZMap.Sample(g_SamplerState, input.TexCoord).xyz;
    float pz = g_TexSSAOViewPos.Sample(g_SamplerState, input.TexCoord).a;
    
    float3 p = getViewPos(input.TexCoord, pz);
    
    float3 randVec = 2.0f * g_TexSSAORandomMap.Sample(g_SamplerState, input.TexCoord).rgb - 1.0f;
    
    float occSum = 0.0f;
    
    for (int i = 0; i < SPHERE_COUNT;i++)
    {
        float3 offset = reflect(SSAO.OffsetVectors[i].xyz, randVec);
        
        float flip = sign(dot(offset, normal));
        
        float3 q = p + flip * SSAO.OcclusionRadius * offset;
        
        float4 projQ = mul(float4(q, 1.0f), SSAO.ViewToTex);
        projQ /= projQ.w;
        projQ.x = projQ.x / 2.0f + 0.5f;
        projQ.y = projQ.y / (-2.0f) + 0.5f;
        
        float rz = g_TexSSAOViewPos.Sample(g_SamplerState, projQ.xy).w;
        
        float3 r = getViewPos(projQ.xy, rz);
        
        float distZ = p.z - r.z;
        float dp = max(dot(normalize(normal), normalize(r - p)), 0);
        float occ = dp * OcculusionFunction(distZ);
        
        occSum += occ;
    }
    
    occSum /= (float)SPHERE_COUNT;
    
    float access = 1.0f - occSum;
    
    output.Diffuse = saturate(pow(access, 4.0f));
    
    return output;
}

// サンプリング数
#define BLUROFFSET_COUNT 16
PSOUTPUT SSAOBlurPS(PSINPUT input)
{
    // ブラーフィルター時のテクセルのオフセット配列
    static float2 BlurOffset16[BLUROFFSET_COUNT] =
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
    };
    
    float ScreenWidth = 960.0f;
    float ScreenHeight = 540.0f;
    
    float Out = 0;
    
    for (int i = 0; i < BLUROFFSET_COUNT; i++)
    {
        float2 offset = BlurOffset16[i] / float2(ScreenWidth, ScreenHeight);
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
    
    matrix invTrnWorld = transpose(inverse(world));
    invTrnWorld[3][0] = 0.0f;
    invTrnWorld[3][1] = 0.0f;
    invTrnWorld[3][2] = 0.0f;
    invTrnWorld[0][3] = 0.0f;
    invTrnWorld[1][3] = 0.0f;
    invTrnWorld[2][3] = 0.0f;
    invTrnWorld[3][3] = 1.0f;
    
    matrix invTrnView = transpose(inverse(View));
    invTrnView[3][0] = 0.0f;
    invTrnView[3][1] = 0.0f;
    invTrnView[3][2] = 0.0f;
    invTrnView[0][3] = 0.0f;
    invTrnView[1][3] = 0.0f;
    invTrnView[2][3] = 0.0f;
    invTrnView[3][3] = 1.0f;
    
    float3x3 wv = mul((float3x3) world, (float3x3) View);
    float4x4 wv4 = float4x4(
        float4(wv[0], 0),
        float4(wv[1], 0),
        float4(wv[2], 0),
        float4(0, 0, 0, 1)
    );
    
    output.Position = mul(input.Position, wvp);
    output.Normal = mul(input.Normal, invTrnWorld);
    output.Normal = normalize(mul(output.Normal, invTrnView));

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
    Out.rgb = (input.Normal.xyz / 2.0f) + 0.5f;
    Out.a = input.ViewPos.z;
    
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
    Out = input.Position.z;
    
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
    Out = input.Position.z;
    
    SSAOPSINSTOUT output;
    output.Diffuse = Out;
    
    return output;
}


 

