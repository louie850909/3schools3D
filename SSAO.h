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
#define OCC_RADIUS 0.8f
#define OCC_FADE_START 0.1f
#define OCC_FADE_END 0.5f
#define SURFACE_EPSILON 0.01f

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
struct SSAO_CONSTANT_BUFFER
{
	XMMATRIX ViewToTex;
	XMFLOAT4 FrustumCorners[4];
	
	float OcclusionRadius;
	float OcclusionFadeStart;
	float OcclusionFadeEnd;
	float SurfaceEpsilon;
};

struct SSAO_OFFSET_VECTORS
{
	XMFLOAT4 OffsetVectors[14];
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
