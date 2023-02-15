//=============================================================================
//
// モデルの処理 [model.h]
// Author : 林　劭羲
//
//=============================================================================
#pragma once

//*********************************************************
// インクルードファイル
//*********************************************************
#include "main.h"
#include "renderer.h"

//*********************************************************
// 構造体
//*********************************************************

// マテリアル構造体

#define MODEL_MAX_MATERIAL		(16)		// １モデルのMaxマテリアル数

struct DX11_MODEL_MATERIAL
{
	MATERIAL					Material;
	ID3D11ShaderResourceView	*Texture;
};

// 描画サブセット構造体
struct DX11_SUBSET
{
	unsigned short	StartIndex;
	unsigned short	IndexNum;
	DX11_MODEL_MATERIAL	Material;
};

struct DX11_MODEL
{
	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;

	VERTEX_3D*		VertexArray;
	unsigned short* IndexArray;

	DX11_SUBSET		*SubsetArray;
	unsigned short	SubsetNum;
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
void LoadModel( char *FileName, DX11_MODEL *Model );
void UnloadModel( DX11_MODEL *Model );
void DrawModel( DX11_MODEL *Model );
void DrawInstanceModel(DX11_MODEL* Model, int InsanceNum, ID3D11Buffer* instanceBuffer, int shaderMode);
void DrawInstanceModelSSAO(DX11_MODEL* Model, int InsanceNum, ID3D11Buffer* instanceBuffer, int pass);

// モデルのマテリアルのディフューズを取得する。Max16個分にしてある
void GetModelDiffuse(DX11_MODEL *Model, XMFLOAT4 *diffuse);

// モデルの指定マテリアルのディフューズをセットする。
void SetModelDiffuse(DX11_MODEL *Model, int mno, XMFLOAT4 diffuse);

bool RayHitModel(DX11_MODEL* Model, XMFLOAT4X4 mtxWorld, XMFLOAT3 rayPos, float rayLength, XMFLOAT3 rayDirection, XMFLOAT3* hitPos, XMFLOAT3* normal);


