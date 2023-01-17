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
    float4 WorldPos : POSITION0;
    float4 LightPos : POSITION1;
};

struct PSOUTPUT
{
    float4 Diffuse : SV_Target;
};
//*****************************************************************************
// 大気描画
//*****************************************************************************
#define PI 3.1415926535897932384626433832795
Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

float Scale(float fcos)
{
    static const float fScaleDepth = 0.25f; // 深度スケール
    float x = 1.0 - fcos;
    return fScaleDepth * exp(-0.00287 + x * (0.459 + x * (3.83 + x * (-6.80 + x * 5.25))));
}

float3 IntersectionPos(float3 dir, float3 a, float radius)
{
    float b = dot(a, dir);
    float c = dot(a, a) - radius * radius;
    float d = max(b * b - c, 0.0);
    
    return a + dir * (-b + sqrt(d));
}

PSOUTPUT SKY(PSINPUT input)
{
    float4 star;
    if (Material.noTexSampling == 0)
    {
        star = g_Texture.Sample(g_SamplerState, input.TexCoord);

        star *= input.Diffuse;
    }
    else
    {
        star = input.Diffuse;
    }
    star = star * Material.Diffuse;
        
    static const float innerRadius = 10000.0f;
    static const float outerRadius = 10250.0f;
    static const float Kr = 0.0025f; // Rayleighの散乱係数
    static const float Km = 0.0010f; // Mieの散乱係数
    
    static const float fSamples = 2.0f; // サンプル数
    static const float3 threePrimary = float3(0.68f, 0.55f, 0.44f); // 三原色
    static const float3 v3InvWavelength = 1.0f / pow(threePrimary, 4.0f); // 逆波長
    
    static const float fESun = 20.0f; // 太陽光の強さ
    static const float fKrESun = Kr * fESun; // Rayleighの散乱係数 * 太陽光の強さ
    static const float fKmESun = Km * fESun; // Mieの散乱係数 * 太陽光の強さ
    
    static const float fKr4PI = Kr * 4.0f * PI; // Rayleighの散乱係数 * 4 * PI
    static const float fKm4PI = Km * 4.0f * PI; // Mieの散乱係数 * 4 * PI
    
    static const float fScale = 1.0f / (outerRadius - innerRadius); // スケール
    static const float fScaleDepth = 0.25f; // 深度スケール
    static const float fScaleOverScaleDepth = fScale / fScaleDepth; // スケール / 深度スケール
    
    static const float g = -0.999f; // Mieの散乱係数
    static const float g2 = g * g; // Mieの散乱係数の2乗
    
    float3 worldPos = input.WorldPos.xyz;
    worldPos = IntersectionPos(normalize(worldPos), float3(0.0, innerRadius, 0.0), outerRadius);
    float3 v3camPos = float3(0.0, innerRadius, 0.0);
    float3 v3LightDir = normalize(Light.Position[0].xyz);
    
    float3 v3Ray = worldPos - v3camPos;
    float fFar = length(v3Ray);
    v3Ray /= fFar;
    
    float3 v3Start = v3camPos;
    float fcameraHeight = length(v3camPos);
    float fStartAngle = dot(v3Ray, v3Start) / fcameraHeight;
    float fStartDepth = exp(fScaleOverScaleDepth * (innerRadius - fcameraHeight));
    float fStartOffset = fStartDepth * Scale(fStartAngle);
    
    float fSampleLength = fFar / fSamples;
    float fScaledLength = fSampleLength * fScale;
    float3 v3SampleRay = v3Ray * fSampleLength;
    float3 v3SamplePoint = v3Start + v3SampleRay * 0.5f;
    
    float3 v3FrontColor = float3(0.0, 0.0, 0.0);
    for (int i = 0; i < (int) fSamples; i++)
    {
        float fHeight = length(v3SamplePoint);
        float fDepth = exp(fScaleOverScaleDepth * (innerRadius - fHeight));
        float fLightAngle = dot(v3LightDir, v3SamplePoint) / fHeight;
        float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
        float fScatter = (fStartOffset + fDepth * (Scale(fLightAngle) - Scale(fCameraAngle)));
        float3 v3Attenuate = exp(-fScatter * (v3InvWavelength * fKr4PI + fKm4PI));
        v3FrontColor += v3Attenuate * (fDepth * fScaledLength);
        v3SamplePoint += v3SampleRay;
    }
    
    float3 c0 = v3FrontColor * (v3InvWavelength * fKrESun);
    float3 c1 = v3FrontColor * fKmESun;
    float3 v3Direction = v3camPos - worldPos;
    
    float fCos = dot(v3LightDir, v3Direction) / length(v3Direction);
    float fCos2 = fCos * fCos;
    
    float rayleighPhase = 0.75 * (1.0 + fCos2);
    float miePhase = 1.5 * ((1.0 - g2) / (2.0 + g2)) * (1.0 + fCos2) / pow(1.0 + g2 - 2.0 * g * fCos, 1.5);
    
    float3 sky = c0 * rayleighPhase + c1 * miePhase;
    
    // Lightの位置が低くになると、星が見えるようにする
    // Lightの位置が高くになると、星がだんだん透明にする
    star.a = saturate((-Light.Position[0].y + 1000.0f) / 2000.0f);
    
    PSOUTPUT output;
    output.Diffuse = 1.0f;
    output.Diffuse.rgb = sky * (1.0f - star.a) + star.rgb * star.a;
    return output;
}