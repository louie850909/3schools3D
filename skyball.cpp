#include "skyball.h"
#include "player.h"

#define	MODEL_SKYBALL		"data/MODEL/skyball.obj"

static SKYBALL g_skyball;
static float g_playerdistX;
static float g_playerdistY;
static float g_playerdistZ;

HRESULT InitSkyBall(void)
{
	LoadModel(MODEL_SKYBALL, &g_skyball.model);
	g_skyball.load = true;

	g_skyball.pos = { 0.0f, 0.0f, 0.0f };
	g_skyball.rot = { 0.0f, 0.0f, 0.0f };
	g_skyball.scl = { 0.01f, 0.01f, 0.01f };

	g_skyball.use = true;

	PLAYER* player = GetPlayer();
	g_playerdistX = g_skyball.pos.x - player->pos.x;
	g_playerdistY = g_skyball.pos.y - player->pos.y;
	g_playerdistZ = g_skyball.pos.z - player->pos.z;

	return S_OK;
}

void UninitSkyBall(void)
{
	// モデルの解放処理
	if (g_skyball.load)
	{
		UnloadModel(&g_skyball.model);
		g_skyball.load = false;
	}
}

void UpdateSkyBall(void)
{
	PLAYER* player = GetPlayer();
	g_skyball.pos.x = player->pos.x + g_playerdistX;
	g_skyball.pos.y = player->pos.y + g_playerdistY;
	g_skyball.pos.z = player->pos.z + g_playerdistZ;
}

void DrawSkyBall(void)
{
	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	SetLightEnable(false);

	// Z比較無し
	SetDepthEnable(false);

	SetAlphaTestEnable(false);

	// フォグ無効
	//SetFogEnable(false);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_skyball.scl.x, g_skyball.scl.y, g_skyball.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_skyball.rot.x, g_skyball.rot.y + XM_PI, g_skyball.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_skyball.pos.x, g_skyball.pos.y, g_skyball.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_skyball.mtxWorld, mtxWorld);

	
	SetShaderMode(SHADER_MODE_SKY);

	// モデル描画
	DrawModel(&g_skyball.model);
	
	SetAlphaTestEnable(false);
	
	SetShaderMode(SHADER_MODE_DEFAULT);

	SetLightEnable(true);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);

	// Z比較有効
	SetDepthEnable(true);

	// フォグ有効
	//SetFogEnable(true);
}
