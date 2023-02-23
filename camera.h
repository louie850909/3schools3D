//=============================================================================
//
// カメラ処理 [camera.h]
// Author : 林　劭羲
//
//=============================================================================
#pragma once


//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	VIEW_ANGLE		(XMConvertToRadians(45.0f))				// ビュー平面の視野角
#define	VIEW_ASPECT		((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)	// ビュー平面のアスペクト比	
#define	VIEW_NEAR_Z		(10.0f)											// ビュー平面のNearZ値
#define	VIEW_FAR_Z		(10000.0f)										// ビュー平面のFarZ値

struct CAMERA
{
	XMFLOAT4X4			mtxView;		// ビューマトリックス
	XMFLOAT4X4			mtxInvView;		// ビューマトリックス
	XMFLOAT4X4			mtxProjection;	// プロジェクションマトリックス
	
	XMFLOAT3			pos;			// カメラの視点(位置)
	XMFLOAT3			at;				// カメラの注視点
	XMFLOAT3			up;				// カメラの上方向ベクトル
	XMFLOAT3			rot;			// カメラの回転
	
	float				len;			// カメラの視点と注視点の距離

};


enum {
	TYPE_FULL_SCREEN,
	TYPE_LEFT_HALF_SCREEN,
	TYPE_RIGHT_HALF_SCREEN,
	TYPE_UP_HALF_SCREEN,
	TYPE_DOWN_HALF_SCREEN,
	TYPE_NONE,

};


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void InitCamera(void);
void UninitCamera(void);
void UpdateCamera(void);
void SetCamera(void);

CAMERA *GetCamera(void);

void SetViewPort(int type);
int GetViewPortType(void);

void SetCameraAT(XMFLOAT3 pos);
void SetCameraPosAtRot(XMFLOAT3 pos, XMFLOAT3 at, XMFLOAT3 rot);
