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

	// 並行光源の設定（世界を照らす光）
	g_Light[0].Position = XMFLOAT3(1379.0f, 2000.0f, 1826.0f);
	g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// 光の向き
	g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// 光の色
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// 並行光源
	g_Light[0].Enable = true;									// このライトをON
	SetLight(0, &g_Light[0]);									// これで設定している



	// フォグの初期化（霧の効果）
	g_Fog.FogStart = 1000.0f;									// 視点からこの距離離れるとフォグがかかり始める
	g_Fog.FogEnd   = 10000.0f;									// ここまで離れるとフォグの色で見えなくなる
	g_Fog.FogColor = XMFLOAT4( 0.9f, 0.9f, 0.9f, 1.0f );		// フォグの色
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);		// 他の場所もチェックする shadow

}


//=============================================================================
// 更新処理
//=============================================================================
void UpdateLight(void)
{
	// ライトの更新
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		if (g_Light[i].Enable == true)
		{
			UpdateLightViewMatrix(i, g_Light[i]);
		}
	}
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
