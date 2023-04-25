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
#define OCC_RADIUS 1.0f
#define OCC_FADEEND 0.25f
#define OCC_FADESTART 0.0f
#define OCC_SURFACEEPSILON 0.13f

enum SSAO_PASS
{
	NormalZMap,
	INSTNormalZMap,
	ViewPosMap,
	INSTViewPosMap,

	SSAO_PASS_MAX,
};

//*****************************************************************************
// 構造体定義
//*****************************************************************************
class SSAO_CONSTANT_BUFFER
{
	public:
		XMMATRIX ViewToTex;
		XMFLOAT4 FrustumCorners[4];
	
		float fadeEND;
		float fadeStart;
		float surfaceEpsilon;
		float Radius;
};

class SSAO_OFFSET_VECTORS
{
	public:
		XMFLOAT4 OffsetVectors[26];
};

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitSSAO();
void UpdateSSAO();
void DrawSSAO();
void UninitSSAO();

void DrawNormalZMap();
void DrawViewPosMap();
void DrawSSAOTex();
void DrawSSAOBlurTex();

ID3D11PixelShader* GetSSAOPixelShader(int pass);
ID3D11VertexShader* GetSSAOVertexShader(int pass);
ID3D11InputLayout* GetSSAOInputLayout(int pass);
