//=============================================================================
//
// 草処理 [grass.cpp]
// Author : 
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "shadow.h"
#include "grass.h"
#include "camera.h"
#include "player.h"
#include "stage.h"
#include "file.h"
#include <thread>


//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define TEXTURE_MAX			(1)				// テクスチャの種類数

#define	GRASS_WIDTH			(16.0f)			// 頂点サイズ
#define	GRASS_HEIGHT		(16.0f)			// 頂点サイズ

#define GRASS_POS_MIN		(-2500.0f)		// 生成位置最小値
#define GRASS_POS_MAX		(2500.0f)			// 生成位置最大値




//*****************************************************************************
// 構造体定義
//*****************************************************************************
typedef struct
{
	MATERIAL	material;		// マテリアル
	bool 		load; 			// テクスチャ読み込みフラグ
	
	bool		bUse;			// 使用しているかどうか

} GRASS;

//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT MakeVertexGrass(void);
void SetGrassHeightA();
void SetGrassHeightB();
void SetGrassHeightC();
void SetGrassHeightD();
void SetGrassHeightE();
void SetGrassHeightF();
void SetGrassHeightG();
void SetGrassHeightH();


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;	// 頂点バッファ
static ID3D11Buffer* g_InstanceBuffer = NULL;	// インスタンスバッファ

static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// テクスチャ情報

static GRASS					g_aGrass;	// 草ワーク
static int						g_TexNo;			// テクスチャ番号
static bool						g_bAlpaTest;		// アルファテストON/OFF
//static int					g_nAlpha;		// アルファテストの閾値

static INSTANCE g_aInstance[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance2[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance3[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance4[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance5[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance6[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance7[MAX_GRASS / 8];			// インスタンスデータ
static INSTANCE g_aInstance8[MAX_GRASS / 8];			// インスタンスデータ

static INSTANCE g_totalInstanceGrass[MAX_GRASS];

static char* g_TextureName[] =
{
	"data/TEXTURE/grass.png",
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitGrass(void)
{
	STAGE* stage = GetStage();
	
	ZeroMemory(&g_aGrass.material, sizeof(g_aGrass.material));
	g_aGrass.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	
	// 頂点情報の作成
	MakeVertexGrass();

	// インスタンスバッファ生成
	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(instanceBufferDesc));
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(INSTANCE) * MAX_GRASS;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_MAPPED_SUBRESOURCE instanceData;
	
	GetDevice()->CreateBuffer(&instanceBufferDesc, NULL, &g_InstanceBuffer);
	GetDeviceContext()->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &instanceData);
	
	INSTANCE* instance = (INSTANCE*)instanceData.pData;

	// BINデータから草の位置を読み込む、ないなら生成する
	FILE* fp = fopen("data/BIN/grass.bin", "rb");
	if (fp != NULL)
	{
		fclose(fp);
		LoadData(SAVE_GRASS);

		for (int i = 0; i < MAX_GRASS; i++)
		{
			instance[i] = g_totalInstanceGrass[i];
		}
	}
	else
	{

		// スレッドを使用して加速する
		std::thread th1(SetGrassHeightA);
		std::thread th2(SetGrassHeightB);
		std::thread th3(SetGrassHeightC);
		std::thread th4(SetGrassHeightD);
		std::thread th5(SetGrassHeightE);
		std::thread th6(SetGrassHeightF);
		std::thread th7(SetGrassHeightG);
		std::thread th8(SetGrassHeightH);

		th1.join();
		th2.join();
		th3.join();
		th4.join();
		th5.join();
		th6.join();
		th7.join();
		th8.join();

		for (int i = 0; i < MAX_GRASS; i++)
		{
			if (i < MAX_GRASS / 8)
			{
				instance[i] = g_aInstance[i];
				g_totalInstanceGrass[i] = g_aInstance[i];
			}
			else if (i < MAX_GRASS / 4)
			{
				instance[i] = g_aInstance2[i - MAX_GRASS / 8];
				g_totalInstanceGrass[i] = g_aInstance2[i - MAX_GRASS / 8];
			}
			else if (i < MAX_GRASS * 3 / 8)
			{
				instance[i] = g_aInstance3[i - MAX_GRASS / 4];
				g_totalInstanceGrass[i] = g_aInstance3[i - MAX_GRASS / 4];
			}
			else if (i < MAX_GRASS / 2)
			{
				instance[i] = g_aInstance4[i - MAX_GRASS * 3 / 8];
				g_totalInstanceGrass[i] = g_aInstance4[i - MAX_GRASS * 3 / 8];
			}
			else if (i < MAX_GRASS * 5 / 8)
			{
				instance[i] = g_aInstance5[i - MAX_GRASS / 2];
				g_totalInstanceGrass[i] = g_aInstance5[i - MAX_GRASS / 2];
			}
			else if (i < MAX_GRASS * 3 / 4)
			{
				instance[i] = g_aInstance6[i - MAX_GRASS * 5 / 8];
				g_totalInstanceGrass[i] = g_aInstance6[i - MAX_GRASS * 5 / 8];
			}
			else if (i < MAX_GRASS * 7 / 8)
			{
				instance[i] = g_aInstance7[i - MAX_GRASS * 3 / 4];
				g_totalInstanceGrass[i] = g_aInstance7[i - MAX_GRASS * 3 / 4];
			}
			else
			{
				instance[i] = g_aInstance8[i - MAX_GRASS * 7 / 8];
				g_totalInstanceGrass[i] = g_aInstance8[i - MAX_GRASS * 7 / 8];
			}
		}
		
		// バイナリファイルに保存
		SaveData(SAVE_GRASS);
	}

	
	
	GetDeviceContext()->Unmap(g_InstanceBuffer, 0);

	// テクスチャ生成
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TextureName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}
	
	g_aGrass.material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	g_aGrass.load = true;
	g_aGrass.bUse = false;

	g_TexNo = 0;

	g_bAlpaTest = true;

	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitGrass(void)
{
	for (int nCntTex = 0; nCntTex < TEXTURE_MAX; nCntTex++)
	{
		if (g_Texture[nCntTex] != NULL)
		{// テクスチャの解放
			g_Texture[nCntTex]->Release();
			g_Texture[nCntTex] = NULL;
		}
	}

	if (g_VertexBuffer != NULL)
	{// 頂点バッファの解放
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	if (g_InstanceBuffer != NULL)
	{
		g_InstanceBuffer->Release();
		g_InstanceBuffer = NULL;
	}
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateGrass(void)
{
	
	
	//for (int nCntGrass = 0; nCntGrass < MAX_GRASS; nCntGrass++)
	//{
	//	if (g_aGrass[nCntGrass].bUse)
	//	{
	//		// 影の位置設定
	//		//SetPositionShadow(g_aGrass[nCntGrass].nIdxShadow, XMFLOAT3(g_aGrass[nCntGrass].pos.x, 0.1f, g_aGrass[nCntGrass].pos.z));
	//	}
	//}

#ifdef _DEBUG
	// アルファテストON/OFF
	if (GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? false : true;
	}

#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawGrass(void)
{
	SetAlphaTestEnable(true);

	// ライティングを無効
	//SetLightEnable(false);
	SetShaderMode(SHADER_MODE_GRASS);

	// 頂点バッファ設定
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* bufferPointer[2];
	
	strides[0] = sizeof(VERTEX_3D);
	strides[1] = sizeof(INSTANCE);
	
	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointer[0] = g_VertexBuffer;
	bufferPointer[1] = g_InstanceBuffer;
	GetDeviceContext()->IASetVertexBuffers(0, 2, bufferPointer, strides, offsets);

	// マテリアル設定
	SetMaterial(g_aGrass.material);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	//SetInstance(g_aInstance, MAX_GRASS);
	GetDeviceContext()->DrawInstanced(4, MAX_GRASS, 0, 0);


	// ライティングを有効に
	//SetLightEnable(true);

	// αテストを無効に
	SetAlphaTestEnable(false);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

void DrawGrassShadowMap(void)
{
	// αテスト設定
	if (g_bAlpaTest == true)
	{
		// αテストを有効に
		SetAlphaTestEnable(true);
	}

	// ライティングを無効
	SetLightEnable(false);
	SetShaderMode(SHADER_MODE_SHADOW_MAP_GRASS);

	// 頂点バッファ設定
	UINT strides[2];
	UINT offsets[2];
	ID3D11Buffer* bufferPointer[2];

	strides[0] = sizeof(VERTEX_3D);
	strides[1] = sizeof(INSTANCE);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointer[0] = g_VertexBuffer;
	bufferPointer[1] = g_InstanceBuffer;
	GetDeviceContext()->IASetVertexBuffers(0, 2, bufferPointer, strides, offsets);

	// マテリアル設定
	SetMaterial(g_aGrass.material);

	// テクスチャ設定
	GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[0]);

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//SetInstance(g_aInstance, MAX_GRASS);
	GetDeviceContext()->DrawInstanced(4, MAX_GRASS, 0, 0);


	// ライティングを有効に
	SetLightEnable(true);

	// αテストを無効に
	SetAlphaTestEnable(false);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

INSTANCE* getGrassInstance()
{
	return &g_totalInstanceGrass[0];
}

//=============================================================================
// 頂点情報の作成
//=============================================================================
HRESULT MakeVertexGrass(void)
{
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);
	

	// 頂点バッファに値をセットする
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	float fWidth = GRASS_WIDTH;
	float fHeight = GRASS_HEIGHT;

	// 頂点座標の設定
	vertex[0].Position = XMFLOAT3(-fWidth / 2.0f, fHeight, 0.0f);
	vertex[1].Position = XMFLOAT3(fWidth / 2.0f, fHeight, 0.0f);
	vertex[2].Position = XMFLOAT3(-fWidth / 2.0f, 0.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(fWidth / 2.0f, 0.0f, 0.0f);

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

	return S_OK;
}

void SetGrassHeightA()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;
	
	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance[i].pos.x, 1000.0f, g_aInstance[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);
		
		if (ans == true)
		{
			g_aInstance[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance[i].pos.y = 20.0f;
		}
		g_aInstance[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightB()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;
	
	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance2[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance2[i].pos.x, 1000.0f, g_aInstance2[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);
		
		if (ans == true)
		{
			g_aInstance2[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance2[i].pos.y = 20.0f;
		}
		g_aInstance2[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance2[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightC()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance3[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance3[i].pos.x, 1000.0f, g_aInstance3[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance3[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance3[i].pos.y = 20.0f;
		}
		g_aInstance3[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance3[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightD()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance4[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance4[i].pos.x, 1000.0f, g_aInstance4[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance4[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance4[i].pos.y = 20.0f;
		}
		g_aInstance4[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance4[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightE()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance5[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance5[i].pos.x, 1000.0f, g_aInstance5[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance5[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance5[i].pos.y = 20.0f;
		}
		g_aInstance5[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance5[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightF()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance6[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance6[i].pos.x, 1000.0f, g_aInstance6[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance6[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance6[i].pos.y = 20.0f;
		}
		g_aInstance6[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance6[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightG()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance7[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance7[i].pos.x, 1000.0f, g_aInstance7[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance7[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance7[i].pos.y = 20.0f;
		}
		g_aInstance7[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance7[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetGrassHeightH()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_GRASS / 8; i++)
	{
		g_aInstance8[i].pos = XMFLOAT4((float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 20.0f, (float)(GRASS_POS_MIN + rand() % (int)(GRASS_POS_MAX - GRASS_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance8[i].pos.x, 1000.0f, g_aInstance8[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance8[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance8[i].pos.y = 20.0f;
		}
		g_aInstance8[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance8[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}