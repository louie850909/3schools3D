#pragma once
#include "model.h"

struct SKYBALL
{
	XMFLOAT3		pos;		// ポリゴンの位置
	XMFLOAT3		rot;		// ポリゴンの向き(回転)
	XMFLOAT3		scl;		// ポリゴンの大きさ(スケール)

	XMFLOAT4X4		mtxWorld;	// ワールドマトリックス

	bool			load;
	DX11_MODEL		model;		// モデル情報
	
	bool			use;
};

HRESULT InitSkyBall(void);
void UninitSkyBall(void);
void UpdateSkyBall(void);
void DrawSkyBall(void);