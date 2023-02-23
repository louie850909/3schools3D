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
SamplerState g_SamplerState   : register(s0);

float3 CreateNormal(float2 xy)
{
    float3 normal;

    normal.xy = xy;
   // max() 入れないとなぜか正しく描画されないときがある
    normal.z = -sqrt(max(1 - normal.x * normal.x - normal.y * normal.y, 0));
    normal = normalize(normal);

    return normal;
}

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

Texture2D g_Texture : register(t0);
PSINPUT NormalZMapVS(VSINPUT input)
{
    PSINPUT output;
    float4x4 wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    
    float3x3 wv = mul((float3x3)World, (float3x3)View);
    float4x4 wv4 = float4x4(
        float4(wv[0],   0),
        float4(wv[1],   0),
        float4(wv[2],   0),
        float4(0, 0, 0, 1)
    );
    
    output.Position = mul(input.Position, wvp);
    output.Normal = input.Normal;
    output.Normal = normalize(mul(input.Normal, inverse(transpose(wv4))));
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
#define SPHERE_COUNT 16
PSOUTPUT SSAOPS(PSINPUT input)
{
    // 周囲ピクセルへのオフセット値
    static float3 SphereArray16[SPHERE_COUNT] =
    {
        float3(0.53812504, 0.18565957, -0.43192)
      , float3(0.13790712, 0.24864247, 0.44301823)
      , float3(0.33715037, 0.56794053, -0.005789503)
      , float3(-0.6999805, -0.04511441, -0.0019965635)
      , float3(0.06896307, -0.15983082, -0.85477847)
      , float3(0.056099437, 0.006954967, -0.1843352)
      , float3(-0.014653638, 0.14027752, 0.0762037)
      , float3(0.010019933, -0.1924225, -0.034443386)
      , float3(-0.35775623, -0.5301969, -0.43581226)
      , float3(-0.3169221, 0.106360726, 0.015860917)
      , float3(0.010350345, -0.58698344, 0.0046293875)
      , float3(-0.08972908, -0.49408212, 0.3287904)
      , float3(0.7119986, -0.0154690035, -0.09183723)
      , float3(-0.053382345, 0.059675813, -0.5411899)
      , float3(0.035267662, -0.063188605, 0.54602677)
      , float3(-0.47761092, 0.2847911, -0.0271716)
    };
    
    float radius = 2.0f;
    float zFar = 10000.0f;
    float zNear = 0.1f;
    float AOPower = 4.7f;
    
    float4 NormalZ = g_TexSSAONormalZMap.Sample(g_SamplerState, input.TexCoord);
    float4 Random = g_TexSSAORandomMap.Sample(g_SamplerState, input.TexCoord);
    
    // 法線ベクトルを作成する
    float3 Normal = CreateNormal(NormalZ.xy);
    // 描画ピクセルのテクセル座標からクリップ空間上の座標を計算
    float4 Pos;
    Pos.xy = (input.TexCoord * float2(2, -2) + float2(-1, 1)) * NormalZ.w;
    Pos.zw = NormalZ.zw;
    
    // カメラ空間上での座標を計算
    float4x4 InvProj = inverse(Projection);
    float4 CameraPos = mul(Pos, InvProj);
    
    float normAO = 0;
    float depthAO = 0;
    
    // サンプリング数分のループ
    for (int i = 0; i < SPHERE_COUNT;i++)
    {
        float3 ray = SphereArray16[i].xyz * radius;
        
        ray = sign(dot(Normal, ray)) * ray;
        
        float4 envPos;
        envPos.xyz = CameraPos.xyz + ray;
        envPos = mul(float4(envPos.xyz, 1), Projection);
        envPos.xy = envPos.xy / envPos.w * float2(0.5, -0.5) + 0.5f;
        
        float4 envNormalZ = g_TexSSAONormalZMap.Sample(g_SamplerState, envPos.xy);
        float3 envNormal = CreateNormal(envNormalZ.xy);
        
        float n = dot(Normal, envNormal) * 0.5 + 0.5;
        
        n += step(NormalZ.z, envNormalZ.z) / zFar;
        normAO += min(n, 1);
        
        depthAO += abs(NormalZ.z - envNormalZ.z) / zFar;
    }
    
    float Out = normAO / (float) SPHERE_COUNT + depthAO;
    Out = pow(abs(Out), AOPower);
    
    PSOUTPUT output;
    output.Diffuse = Out;
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
    
    float3x3 wv = mul((float3x3) world, (float3x3) View);
    float4x4 wv4 = float4x4(
        float4(wv[0], 0),
        float4(wv[1], 0),
        float4(wv[2], 0),
        float4(0, 0, 0, 1)
    );
    
    output.Position = mul(input.Position, wvp);
    output.Normal = normalize(mul(input.Normal, transpose(inverse(wv4))));

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

SSAOPSINSTIN SSAO_GRASSVS(SSAOVSINSTIN input)
{
    SSAOPSINSTIN output;
    matrix wd;
    matrix wvp;
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
    wvp = mul(wd, View);
    wvp = mul(wvp, Projection);
    // xz bias for grass wave
    float x = sin(Time.x + input.inInstancePos.x) * 2.0f;
    float z = sin(Time.x + input.inInstancePos.x) * 2.0f;
    
    if (input.TexCoord.y == 0.0f)
    {
        output.Position = mul(float4(input.Position.x + x, input.Position.y, input.Position.z + z, 1.0f), wvp);
    }
    else
    {
        output.Position = mul(input.Position, wvp);
    }
	
    output.Normal = mul(input.Normal, wd);

    output.TexCoord = input.TexCoord;
	
    output.ViewPos = output.Position;

    output.Diffuse = input.Diffuse;
    
    output.inInstancePos = input.inInstancePos;
    
    output.inInstanceScl = input.inInstanceScl;
    
    output.inInstanceRot = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    return output;
}

SSAOPSINSTOUT SSAO_GRASSPS(SSAOPSINSTIN input)
{
    matrix wd;
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
    
    float4 texColor = g_Texture.Sample(g_SamplerState, input.TexCoord);
    if (texColor.a < 0.1)
    {
        discard;
    }
    
    float4x4 Wxv = mul(wd, View);
    
    float4 Out;
    Out.xy = normalize(mul(float4(input.Normal.xyz, 0), Wxv).xyz).xy;
    Out.zw = input.ViewPos.zw;
    
    SSAOPSINSTOUT output;
    output.Diffuse = Out;
    return output;
}


 

