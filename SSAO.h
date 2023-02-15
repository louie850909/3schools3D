//=============================================================================
//
// SSAO処理 [SSAO.h]
// Author : 林　劭羲
//
//=============================================================================
#pragma once

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "main.h"
#include "renderer.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
enum SSAO_PASS
{
	NormalZMap,
	INSTNormalZMap,
	GrassNormalZMap,

	SSAO_PASS_MAX,
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitSSAO();
void UpdateSSAO();
void DrawSSAO();
void UninitSSAO();

void DrawNormalZMap();
void DrawSSAOTex();
void DrawSSAOBlurTex();

ID3D11PixelShader* GetSSAOPixelShader(int pass);
ID3D11VertexShader* GetSSAOVertexShader(int pass);
ID3D11InputLayout* GetSSAOInputLayout(int pass);
