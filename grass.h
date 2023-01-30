//=============================================================================
//
// 木処理 [grass.h]
// Author : 
//
//=============================================================================
#pragma once

#define	MAX_GRASS			(50000)			// 草最大数
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitGrass(void);
void UninitGrass(void);
void UpdateGrass(void);
void DrawGrass(void);
void DrawGrassShadowMap(void);
void DrawGrassSSAO(int pass);

INSTANCE* getGrassInstance();


