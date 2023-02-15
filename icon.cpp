//=============================================================================
//
// ミニマップ用アイコン処理 [icon.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "icon.h"
#include "player.h"
#include "enemy.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_WIDTH				(128)	// キャラサイズ
#define TEXTURE_HEIGHT				(128)	// 
#define TEXTURE_MAX					(2)		// テクスチャの数

//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;							// 頂点情報
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/circle.png",
	"data/TEXTURE/cross.png",
};

static ICON g_PlayerIcon;	// プレイヤーのアイコン
static ICON g_enemyIcon;	// 敵のアイコン

static bool g_Load;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitIcon(void)
{
	//テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// プレイヤーのアイコン
	g_PlayerIcon.w = TEXTURE_WIDTH;
	g_PlayerIcon.h = TEXTURE_HEIGHT;
	g_PlayerIcon.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_PlayerIcon.texNo = 0;
	g_PlayerIcon.use = true;

	// 敵のアイコン
	g_enemyIcon.w = TEXTURE_WIDTH;
	g_enemyIcon.h = TEXTURE_HEIGHT;
	g_enemyIcon.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_enemyIcon.texNo = 1;
	g_enemyIcon.use = true;

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(-g_PlayerIcon.w / 2.0f, 0.0f, g_PlayerIcon.h / 2.0f);
	vertex[1].Position = XMFLOAT3( g_PlayerIcon.w / 2.0f, 0.0f, g_PlayerIcon.h / 2.0f);
	vertex[2].Position = XMFLOAT3(-g_PlayerIcon.w / 2.0f, 0.0f,	-g_PlayerIcon.h / 2.0f);
	vertex[3].Position = XMFLOAT3(g_PlayerIcon.w / 2.0f, 0.0f,	-g_PlayerIcon.h / 2.0f);

	// 拡散光の設定
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// 法線の設定
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	// テクスチャ座標の設定
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	
	g_Load = TRUE;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitIcon(void)
{
	if (!g_Load) return;

	// 頂点バッファの開放
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	
	// テクスチャの開放
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i] != NULL)
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateIcon(void)
{
	if (g_PlayerIcon.use)
	{
		PLAYER* player = GetPlayer();
		g_PlayerIcon.pos = XMFLOAT3(player->pos.x, player->pos.y + 200.0f, player->pos.z);
	}

	if (g_enemyIcon.use)
	{
		ENEMY* enemy = GetEnemy();
		g_enemyIcon.pos = XMFLOAT3(enemy->pos.x, enemy->pos.y + 400.0f, enemy->pos.z);
	}
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawIcon(void)
{
	if (!g_Load) return;

	if (g_PlayerIcon.use)
	{
		// ワールド行列の設定
		XMMATRIX mtxWorld, mtxScl, mtxTrans;
		mtxWorld = XMMatrixIdentity();
		mtxScl = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		mtxTrans = XMMatrixTranslation(g_PlayerIcon.pos.x, g_PlayerIcon.pos.y, g_PlayerIcon.pos.z);
		mtxWorld = XMMatrixMultiply(mtxScl, mtxWorld);
		mtxWorld = XMMatrixMultiply(mtxTrans, mtxWorld);

		// ワールド行列を定数バッファにセット
		SetWorldMatrix(&mtxWorld);

		// 頂点バッファをセット
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// テクスチャをセット
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_PlayerIcon.texNo]);

		// プリミティブトポロジ設定
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ポリゴンの描画
		GetDeviceContext()->Draw(4, 0);
	}

	if (g_enemyIcon.use)
	{
		// ワールド行列の設定
		XMMATRIX mtxWorld, mtxScl, mtxTrans;
		mtxWorld = XMMatrixIdentity();
		mtxScl = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		mtxTrans = XMMatrixTranslation(g_enemyIcon.pos.x, g_enemyIcon.pos.y, g_enemyIcon.pos.z);
		mtxWorld = XMMatrixMultiply(mtxScl, mtxWorld);
		mtxWorld = XMMatrixMultiply(mtxTrans, mtxWorld);

		// ワールド行列を定数バッファにセット
		SetWorldMatrix(&mtxWorld);

		// 頂点バッファをセット
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// テクスチャをセット
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_enemyIcon.texNo]);

		// プリミティブトポロジ設定
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ポリゴンの描画
		GetDeviceContext()->Draw(4, 0);
	}
}
