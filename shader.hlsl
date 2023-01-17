

//*****************************************************************************
// 定数バッファ
//*****************************************************************************

// マトリクスバッファ
cbuffer WorldBuffer : register( b0 )
{
	matrix World;
}

cbuffer ViewBuffer : register( b1 )
{
	matrix View;
}

cbuffer ProjectionBuffer : register( b2 )
{
	matrix Projection;
}

// マテリアルバッファ
struct MATERIAL
{
	float4		Ambient;
	float4		Diffuse;
	float4		Specular;
	float4		Emission;
	float		Shininess;
	int			noTexSampling;
	float		Dummy[2];//16byte境界用
};

cbuffer MaterialBuffer : register( b3 )
{
	MATERIAL	Material;
}

// ライト用バッファ
struct LIGHT
{
	float4		Direction[5];
	float4		Position[5];
	float4		Diffuse[5];
	float4		Ambient[5];
	float4		Attenuation[5];
	int4		Flags[5];
	int			Enable;
	int			Dummy[3];//16byte境界用
};

cbuffer LightBuffer : register( b4 )
{
	LIGHT		Light;
}

struct FOG
{
	float4		Distance;
	float4		FogColor;
	int			Enable;
	float		Dummy[3];//16byte境界用
};

// フォグ用バッファ
cbuffer FogBuffer : register( b5 )
{
	FOG			Fog;
}

// 縁取り用バッファ
cbuffer Fuchi : register(b6)
{
	int			fuchi;
	int			fill[3];
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
    float4 Normal   : NORMAL0;
    float4 Diffuse  : COLOR0;
    float2 TexCoord : TEXCOORD0;
};

struct VSINSTINPUT
{
    float4 Position		 : POSITION0;
    float4 Normal		 : NORMAL0;
    float4 Diffuse		 : COLOR0;
    float2 TexCoord		 : TEXCOORD0;
    float4 inInstancePos : INSTPOS0;
    float4 inInstanceScl : INSTSCL0;
    float4 inInstanceRot : INSTROT0;
};

struct PSINPUT
{
    float4 Position : SV_POSITION;
    float4 Normal	: NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Diffuse	: COLOR0;
    float4 WorldPos : POSITION0;
    float4 LightPos : POSITION1;
};

struct PSOUTPUT
{
    float4 Diffuse : SV_Target;
};
	
//=============================================================================
// 頂点シェーダ
//=============================================================================
PSINPUT VertexShaderPolygon( VSINPUT input)
{
    PSINPUT output;
	
    matrix wvp;
	
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    output.Position = mul(input.Position, wvp);

    output.Normal = normalize(mul(float4(input.Normal.xyz, 0.0f), World));

    output.TexCoord = input.TexCoord;

    output.WorldPos = mul(input.Position, World);

    output.Diffuse = input.Diffuse;
	
    output.LightPos = mul(input.Position, World);
    output.LightPos = mul(output.LightPos, LightMatrix.LightView[0]);
    output.LightPos = mul(output.LightPos, LightMatrix.LightProjection[0]);
	
    return output;
}



//*****************************************************************************
// グローバル変数
//*****************************************************************************
Texture2D		g_Texture : register(t0);
Texture2D		g_TexDepth : register(t1);
SamplerState	g_SamplerState : register( s0 );
SamplerState g_SamplerStateClamp : register(s1);


//=============================================================================
// ピクセルシェーダ
//=============================================================================
PSOUTPUT PixelShaderPolygon(PSINPUT input )
{
	float4 color;
    PSOUTPUT output;

	if (Material.noTexSampling == 0)
	{
		color = g_Texture.Sample(g_SamplerState, input.TexCoord);
		
        if (color.a < 0.5f)
        {
            discard;
        }

		color *= input.Diffuse;
	}
	else
	{
		color = input.Diffuse;
	}

	if (Light.Enable == 0)
	{
		color = color * Material.Diffuse;
	}
	else
	{
		float4 tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 outColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

		for (int i = 0; i < 5; i++)
		{
			float3 lightDir;
			float light;

			if (Light.Flags[i].y == 1)
			{
				if (Light.Flags[i].x == 1)
				{
					lightDir = normalize(Light.Direction[i].xyz);
					light = dot(lightDir, input.Normal.xyz);

					light = 0.5 - 0.5 * light;
					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];
				}
				else if (Light.Flags[i].x == 2)
				{
					lightDir = normalize(Light.Position[i].xyz - input.WorldPos.xyz);
					light = dot(lightDir, input.Normal.xyz);

					tempColor = color * Material.Diffuse * light * Light.Diffuse[i];

					float distance = length(input.WorldPos - Light.Position[i]);

					float att = saturate((Light.Attenuation[i].x - distance) / Light.Attenuation[i].x);
					tempColor *= att;
				}
				else
				{
					tempColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
				}

				outColor += tempColor;
			}
		}

		color = outColor;
		color.a = input.Diffuse.a * Material.Diffuse.a;
	}

	//フォグ
	if (Fog.Enable == 1)
	{
		float z = input.Position.z*input.Position.w;
		float f = (Fog.Distance.y - z) / (Fog.Distance.y - Fog.Distance.x);
		f = saturate(f);
		output.Diffuse = f * color + (1 - f)*Fog.FogColor;
		output.Diffuse.a = color.a;
	}
	else
	{
		output.Diffuse = color;
	}

	//縁取り
	if (fuchi == 1)
	{
		float angle = dot(normalize(input.WorldPos.xyz - Camera.xyz), normalize(input.Normal));
		//if ((angle < 0.5f)&&(angle > -0.5f))
		if (angle > -0.3f)
		{
			output.Diffuse.rb  = 1.0f;
			output.Diffuse.g = 0.0f;			
		}
	}
	
	// 影
    if (Light.Enable == 1 && Light.Position[0].y >= -200.0f)
    {
        float bias = 0.0001f;
        float2 shadowTexCoord;
        float shadowMapDepth;
        float depth;
		
        float shadowMapSizeX = 1024.0f * 5.0f;
        float shadowMapSizeY = 1024.0f * 5.0f;
		
        shadowTexCoord = input.LightPos.xy / input.LightPos.w;
        shadowTexCoord = shadowTexCoord * float2(0.5f, -0.5f) + 0.5;
		
        if (saturate(shadowTexCoord.x) == shadowTexCoord.x && saturate(shadowTexCoord.y) == shadowTexCoord.y)
        {
            shadowMapDepth = g_TexDepth.Sample(g_SamplerState, shadowTexCoord).r;
            depth = input.LightPos.z / input.LightPos.w;
			
            if (shadowMapDepth + bias < depth)
            {
                output.Diffuse.rgb *= 0.5f;
            }
        }
    }
	
    return output;
}

//=============================================================================
// インスタンシンVSシェーダ
//=============================================================================
[earlydepthstencil]
PSINPUT VertexShaderInstancing(VSINSTINPUT input)
{
    PSINPUT output;
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
	
    output.Normal = normalize(mul(float4(input.Normal.xyz, 0.0f), world));

    output.TexCoord = input.TexCoord;

    output.WorldPos = mul(input.inInstancePos, world);

    output.Diffuse = input.Diffuse;
	
    output.LightPos = mul(input.Position, world);
    output.LightPos = mul(output.LightPos, LightMatrix.LightView[0]);
    output.LightPos = mul(output.LightPos, LightMatrix.LightProjection[0]);
	
    return output;
}
