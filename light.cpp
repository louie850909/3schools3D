//=============================================================================
//
// ライト処理 [light.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define LIGHT_SUNRISE_POS_X		(5000.3f)		// 日の出のX位置
#define LIGHT_SUNRISE_POS_Y		(-869.3f)		// 日の出のY位置
#define LIGHT_SUNRISE_POS_Z		(0.0f)			// 日の出のZ位置
#define LIGHT_SUNSET_POS_X		(-2245.3f)		// 日の入りのX位置
#define LIGHT_SUNSET_POS_Y		(-869.3f)		// 日の入りのY位置
#define LIGHT_SUNSET_POS_Z		(5000.0f)		// 日の入りのZ位置
#define LIGHT_NOON_POS_X		(1377.35f)		// 昼のX位置
#define LIGHT_NOON_POS_Y		(5000.0f)		// 昼のY位置
#define LIGHT_NOON_POS_Z		(2500.0f)		// 昼のZ位置
#define LIGHT_MIDNINGT_POS_X	(1377.35f)		// 夜のX位置
#define LIGHT_MIDNINGT_POS_Y	(-5000.0f)		// 夜のY位置
#define LIGHT_MIDNINGT_POS_Z	(2500.0f)		// 夜のZ位置

#define SUN_MOVING_TIME			(600.0f)		// 日の移動時間（フレーム）
enum TIME
{
	SUNRISE,
	NOON,
	SUNSET,
	MIDNIGHT,

	TIME_NUM,
};
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void UpdateLightViewMatrix(int index, LIGHT light);

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

static	BOOL	g_FogEnable = true;

static int		g_nowTime = SUNRISE;
static float	g_time = -1;

XMFLOAT3 RiseToNoon = XMFLOAT3(
	(LIGHT_NOON_POS_X - LIGHT_SUNRISE_POS_X) / SUN_MOVING_TIME,
	(LIGHT_NOON_POS_Y - LIGHT_SUNRISE_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_NOON_POS_Z - LIGHT_SUNRISE_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 NoonToSet = XMFLOAT3(
	(LIGHT_SUNSET_POS_X - LIGHT_NOON_POS_X) / SUN_MOVING_TIME,
	(LIGHT_SUNSET_POS_Y - LIGHT_NOON_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_SUNSET_POS_Z - LIGHT_NOON_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 SetToMidnight = XMFLOAT3(
	(LIGHT_MIDNINGT_POS_X - LIGHT_SUNSET_POS_X) / SUN_MOVING_TIME,
	(LIGHT_MIDNINGT_POS_Y - LIGHT_SUNSET_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_MIDNINGT_POS_Z - LIGHT_SUNSET_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 MidnightToRise = XMFLOAT3(
	(LIGHT_SUNRISE_POS_X - LIGHT_MIDNINGT_POS_X) / SUN_MOVING_TIME,
	(LIGHT_SUNRISE_POS_Y - LIGHT_MIDNINGT_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_SUNRISE_POS_Z - LIGHT_MIDNINGT_POS_Z) / SUN_MOVING_TIME
);

//=============================================================================
// 初期化処理
//=============================================================================
void InitLight(void)
{
	//ライト初期化
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// 減衰距離
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ライトのタイプ
		g_Light[i].Enable = false;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	switch (GetMode())
	{
	case MODE_TITLE:
		g_nowTime = SUNRISE;
		
		// 並行光源の設定（世界を照らす光）
		g_Light[0].Position = XMFLOAT3(LIGHT_SUNRISE_POS_X, LIGHT_SUNRISE_POS_Y, LIGHT_SUNRISE_POS_Z);
		g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// 光の向き
		g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 光の色
		g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
		g_Light[0].Enable = true;									// このライトをON
		SetLight(0, &g_Light[0]);									// これで設定している

		// フォグの初期化（霧の効果）
		g_Fog.FogStart = 1000.0f;									// 視点からこの距離離れるとフォグがかかり始める
		g_Fog.FogEnd = 10000.0f;									// ここまで離れるとフォグの色で見えなくなる
		g_Fog.FogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);		// フォグの色
		SetFog(&g_Fog);
		SetFogEnable(g_FogEnable);		// 他の場所もチェックする shadow
		break;
		
	case MODE_GAME:
		g_nowTime = NOON;
		
		// 並行光源の設定（世界を照らす光）
		g_Light[0].Position = XMFLOAT3(LIGHT_NOON_POS_X, LIGHT_NOON_POS_Y, LIGHT_NOON_POS_Z);
		g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// 光の向き
		g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 光の色
		g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
		g_Light[0].Enable = true;									// このライトをON
		SetLight(0, &g_Light[0]);									// これで設定している

		// フォグの初期化（霧の効果）
		g_Fog.FogStart = 1000.0f;									// 視点からこの距離離れるとフォグがかかり始める
		g_Fog.FogEnd = 10000.0f;									// ここまで離れるとフォグの色で見えなくなる
		g_Fog.FogColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);		// フォグの色
		SetFog(&g_Fog);
		SetFogEnable(g_FogEnable);		// 他の場所もチェックする shadow
	}
	
	g_time = -1;
}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{
	g_time++;
	switch (GetMode())
	{
	case MODE_TITLE:
		
		switch (g_nowTime)
		{
		case SUNRISE:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_SUNRISE_POS_X + g_time * RiseToNoon.x;
				g_Light[0].Position.y = LIGHT_SUNRISE_POS_Y + g_time * RiseToNoon.y;
				g_Light[0].Position.z = LIGHT_SUNRISE_POS_Z + g_time * RiseToNoon.z;

				g_Fog.FogColor.x += 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.y += 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.z += 1.0f / SUN_MOVING_TIME;
			}
			else
			{
				g_time = -1;
				g_nowTime = NOON;
			}
			break;

		case NOON:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_NOON_POS_X + g_time * NoonToSet.x;
				g_Light[0].Position.y = LIGHT_NOON_POS_Y + g_time * NoonToSet.y;
				g_Light[0].Position.z = LIGHT_NOON_POS_Z + g_time * NoonToSet.z;

				g_Fog.FogColor.x -= 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.y -= 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.z -= 1.0f / SUN_MOVING_TIME;
			}
			else
			{
				g_time = -1;
				g_nowTime = SUNSET;
			}
			break;

		case SUNSET:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_SUNSET_POS_X + g_time * SetToMidnight.x;
				g_Light[0].Position.y = LIGHT_SUNSET_POS_Y + g_time * SetToMidnight.y;
				g_Light[0].Position.z = LIGHT_SUNSET_POS_Z + g_time * SetToMidnight.z;
			}
			else
			{
				g_time = -1;
				g_nowTime = MIDNIGHT;
			}
			break;

		case MIDNIGHT:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_MIDNINGT_POS_X + g_time * MidnightToRise.x;
				g_Light[0].Position.y = LIGHT_MIDNINGT_POS_Y + g_time * MidnightToRise.y;
				g_Light[0].Position.z = LIGHT_MIDNINGT_POS_Z + g_time * MidnightToRise.z;
			}
			else
			{
				g_time = -1;
				g_nowTime = SUNRISE;
			}
			break;
		}
		break;

	case MODE_GAME:
		break;
	}

	// ライトの更新
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		if (g_Light[i].Enable == true)
		{
			UpdateLightViewMatrix(i, g_Light[i]);
		}
	}
	SetLightData(0, &g_Light[0]);
	SetFog(&g_Fog);
}


//=============================================================================
// ライトの設定
// Typeによってセットするメンバー変数が変わってくる
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// フォグの設定
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}

void UpdateLightViewMatrix(int index, LIGHT light)
{
	XMMATRIX LightView = XMMatrixIdentity();
	XMMATRIX LightProjection = XMMatrixIdentity();

	LIGHT_MATRIX lightMatrix;

	switch (light.Type)
	{
	case LIGHT_TYPE_DIRECTIONAL:
		// 並行光源の場合は、カメラの位置を光源の位置とする
		XMVECTOR eyePos = XMLoadFloat3(&light.Position);
		XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightView = XMMatrixLookAtLH(eyePos, lookAt, up);
		LightProjection = XMMatrixOrthographicLH(10000.0f, 10000.0f, VIEW_NEAR_Z, VIEW_FAR_Z * 2);

		lightMatrix.LightView = LightView;
		lightMatrix.LightProjection = LightProjection;
		SetLightMatrix(index, &lightMatrix);
		break;

	case LIGHT_TYPE_POINT:
		// 点光源の場合は、カメラの位置を光源の位置とする
		eyePos = XMLoadFloat3(&light.Position);
		lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightView = XMMatrixLookAtLH(eyePos, lookAt, up);
		LightProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

		lightMatrix.LightView = LightView;
		lightMatrix.LightProjection = LightProjection;
		SetLightMatrix(index, &lightMatrix);
		break;
	}
}
