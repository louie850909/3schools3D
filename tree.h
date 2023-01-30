//=============================================================================
//
// 木処理 [tree.h]
// Author : 
//
//=============================================================================
#pragma once

#define	MAX_TREE			(2000)			// 木最大数
//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitTree(void);
void UninitTree(void);
void UpdateTree(void);
void DrawTree(void);
void DrawTreeShadowMap(void);
void DrawTreeSSAO(int pass);

INSTANCE* getTreeInstance();

