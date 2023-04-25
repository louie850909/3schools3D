//=============================================================================
//
// 地形処理 [stage.h]
// Author : 林　劭羲
//
//=============================================================================
#pragma once

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "main.h"
#include "renderer.h"
#include "model.h"

//*****************************************************************************
// 構造体
//*****************************************************************************
class STAGE
{
	public:
		XMFLOAT3		pos;		// ポリゴンの位置
		XMFLOAT3		rot;		// ポリゴンの向き(回転)
		XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)

		XMFLOAT4X4		mtxWorld;	// ワールドマトリックス

		bool			load;
		DX11_MODEL		model;		// モデル情報

		bool			use;
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitStage(void);
void UninitStage(void);
void UpdateStage(void);
void DrawStage(void);

STAGE* GetStage(void);