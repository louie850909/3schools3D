//=============================================================================
//
// 地形処理 [stage.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "stage.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_SKYBALL		"data/MODEL/landscape.obj"

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static STAGE g_stage;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitStage(void)
{
	LoadModel(MODEL_SKYBALL, &g_stage.model);
	g_stage.load = true;

	g_stage.pos = { 0.0f, 0.0f, 0.0f };
	g_stage.rot = { 0.0f, 0.0f, 0.0f };
	g_stage.scl = { 1.0f, 1.0f, 1.0f };

	g_stage.use = true;

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_stage.scl.x, g_stage.scl.y, g_stage.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_stage.rot.x, g_stage.rot.y + XM_PI, g_stage.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 位置を反映
	mtxTranslate = XMMatrixTranslation(g_stage.pos.x, g_stage.pos.y, g_stage.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	XMStoreFloat4x4(&g_stage.mtxWorld, mtxWorld);

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitStage(void)
{
	// モデルの解放処理
	if (g_stage.load)
	{
		UnloadModel(&g_stage.model);
		g_stage.load = false;
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateStage(void)
{
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawStage(void)
{
	XMMATRIX mtxWorld = XMLoadFloat4x4(&g_stage.mtxWorld);

	// ワールドマトリックスを設定
	SetWorldMatrix(&mtxWorld);
	
	

	// モデルの描画
	DrawModel(&g_stage.model);
}

//=============================================================================
// モデルの取得
//=============================================================================
STAGE* GetStage(void)
{
	return &g_stage;
}
