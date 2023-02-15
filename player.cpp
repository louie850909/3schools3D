//=============================================================================
//
// モデル処理 [player.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "light.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "shadow.h"
#include "bullet.h"
#include "debugproc.h"
#include "meshfield.h"
#include "enemy.h"
#include "stage.h"
#include "fade.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_PLAYER			"data/MODEL/cone.obj"				// 読み込むモデル名
#define	MODEL_PLAYER_LEFT_HAND	"data/MODEL/human_left_hand.obj"	// 読み込むモデル名
#define	MODEL_PLAYER_RIGHT_HAND	"data/MODEL/hand_and_sword.obj"		// 読み込むモデル名

#define	VALUE_MOVE			(2.0f)							// 移動量
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// 回転量

#define PLAYER_SHADOW_SIZE	(0.4f)							// 影の大きさ
#define PLAYER_OFFSET_Y		(30.0f)							// プレイヤーの足元をあわせる

#define PLAYER_PARTS_MAX	(2)								// プレイヤーのパーツの数

#define INIT_POS_X			(1095.0f)							// 初期位置(X座標)
#define INIT_POS_Y			(680.0f)							// 初期位置(Y座標)
#define INIT_POS_Z			(1054.0f)							// 初期位置(Z座標)



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static PLAYER		g_Player;						// プレイヤー

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// プレイヤーのパーツ用

static float		roty = 0.0f;

static LIGHT		g_Light;

// プレイヤーの階層アニメーションデータ
// プレイヤーの歩いているアニメデータ
static INTERPOLATION_DATA move_lefthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.0f, -5.3f, 9.6f), XMFLOAT3(-0.2f, -0.1f, 1.3f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(-5.0f, -5.3f, 9.6f), XMFLOAT3(-1.5f, -0.1f, 1.3f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(-5.0f, -5.3f, 9.6f), XMFLOAT3(-0.2f, -0.1f, 1.3f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	//{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};

static INTERPOLATION_DATA move_righthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.0f, -5.3f, 9.6f), XMFLOAT3(-0.6f, -1.3f, -6.5f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(5.0f, -5.3f, 9.6f), XMFLOAT3(0.2, -0.5f, -7.6f),    XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	{ XMFLOAT3(5.0f, -5.3f, 9.6f), XMFLOAT3(-0.6f, -1.3f, -6.5f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	//{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),         XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(-20.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};

static INTERPOLATION_DATA idle_lefthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(-5.0f, -5.3f, 9.6f), XMFLOAT3(-1.5f, -0.1f, 1.3f),      XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI / 2, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, XM_PI / 2, 0.0f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};

static INTERPOLATION_DATA idle_righthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(5.0f, -5.3f, 9.6f), XMFLOAT3(-0.6f, -1.3f, -6.5f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 30 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI / 2, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, XM_PI / 2, 0.0f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};


// プレイヤーの走っているアニメデータ
static INTERPOLATION_DATA run_lefthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),       XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI / 2, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, XM_PI / 2, 0.0f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};

static INTERPOLATION_DATA run_righthand_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),       XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, -XM_PI / 2, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 240 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, XM_PI / 2, 0.0f),  XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },
	//{ XMFLOAT3(0.0f, 10.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f),	     XMFLOAT3(1.0f, 1.0f, 1.0f), 120 },

};







//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Player.model);

	g_Player.pos = XMFLOAT3(INIT_POS_X, PLAYER_OFFSET_Y+INIT_POS_Y, INIT_POS_Z);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.state = MOVE;

	g_Player.spd = 0.0f;			// 移動スピードクリア

	g_Player.use = TRUE;			// true:生きてる
	g_Player.size = PLAYER_SIZE;	// 当たり判定の大きさ

	// ここでプレイヤー用の影を作成している
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ↑
	//        このメンバー変数が生成した影のIndex番号

	// キーを押した時のプレイヤーの向き
	roty = 0.0f;

	g_Player.parent = NULL;			// 本体（親）なのでNULLを入れる


	// 階層アニメーションの初期化
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = TRUE;

		// 位置・回転・スケールの初期設定
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// 親子関係
		g_Parts[i].parent = &g_Player;		// ← ここに親のアドレスを入れる
	//	g_Parts[腕].parent= &g_Player;		// 腕だったら親は本体（プレイヤー）
	//	g_Parts[手].parent= &g_Paerts[腕];	// 指が腕の子供だった場合の例

		// 階層アニメーション用のメンバー変数の初期化
		//g_Parts[i].tbl_adr = move_tbl;	// 再生するアニメデータの先頭アドレスをセット
		g_Parts[i].move_time = 0.0f;	// 実行時間をクリア
	}
	// パーツの読み込み
	g_Parts[0].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_HAND, &g_Parts[0].model);
	g_Parts[0].parent = &g_Player;		// 
	g_Parts[0].tbl_adr = move_righthand_tbl;		// 再生するアニメデータの先頭アドレスをセット
	
	// パーツの読み込み
	g_Parts[1].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_HAND, &g_Parts[1].model);
	g_Parts[1].parent = &g_Player;		// 
	g_Parts[1].tbl_adr = move_lefthand_tbl;		// 再生するアニメデータの先頭アドレスをセット


	// クォータニオンの初期化
	XMStoreFloat4(&g_Player.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitPlayer(void)
{
	// モデルの解放処理
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

	// パーツの解放処理
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		if (g_Parts[i].load == TRUE)
		{
			// パーツの解放処理
			UnloadModel(&g_Parts[i].model);
			g_Parts[i].load = FALSE;
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdatePlayer(void)
{
	g_Player.state = IDLE;
	XMFLOAT3 prePos = g_Player.pos;		// 前回の座標を保存
	
	CAMERA *cam = GetCamera();

	g_Player.spd *= 0.9f;
	g_Player.rot.y = cam->rot.y;
	g_Player.rot.x = cam->rot.x;

	// レイキャストして足元の高さを求める
	XMFLOAT3 HitPosition;		// 交点
	XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
	STAGE* stage = GetStage();
	bool ans = RayHitModel(&stage->model, stage->mtxWorld, g_Player.pos, 300.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &HitPosition, &Normal);
	if (ans)
	{
		g_Player.pos.y = HitPosition.y + PLAYER_OFFSET_Y;
	}

	// fps移動処理
	if (GetKeyboardPress(DIK_A))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.x -= g_Player.spd;

		g_Player.pos.x -= cosf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z += sinf(g_Player.rot.y) * g_Player.spd;
	}

	if (IsButtonTriggered(0, BUTTON_LEFT))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.x -= g_Player.spd;

		g_Player.pos.x -= cosf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z += sinf(g_Player.rot.y) * g_Player.spd;
	}
	
	if (GetKeyboardPress(DIK_D))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.x += g_Player.spd;
		
		g_Player.pos.x += cosf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= sinf(g_Player.rot.y) * g_Player.spd;
	}

	if (IsButtonTriggered(0, BUTTON_RIGHT))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.x += g_Player.spd;

		g_Player.pos.x += cosf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= sinf(g_Player.rot.y) * g_Player.spd;
	}
	
	if (GetKeyboardPress(DIK_W))
	{
		g_Player.state = MOVE;
		g_Player.move_time += 0.1f;
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.z += g_Player.spd;
		
		g_Player.pos.x += sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z += cosf(g_Player.rot.y) * g_Player.spd;

		// 走るの時にｙ方向を振動させる
		g_Player.pos.y += sinf(g_Player.move_time * 3.0f) * 1.0f;
	}

	if (IsButtonTriggered(0, BUTTON_UP))
	{
		g_Player.state = MOVE;
		g_Player.move_time += 0.1f;
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.z += g_Player.spd;

		g_Player.pos.x += sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z += cosf(g_Player.rot.y) * g_Player.spd;

		// 走るの時にｙ方向を振動させる
		g_Player.pos.y += sinf(g_Player.move_time * 3.0f) * 1.0f;
	}
	
	if (GetKeyboardPress(DIK_S))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.z -= g_Player.spd;

		g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;
	}

	if (IsButtonTriggered(0, BUTTON_DOWN))
	{
		g_Player.spd = VALUE_MOVE;
		//g_Player.pos.z -= g_Player.spd;

		g_Player.pos.x -= sinf(g_Player.rot.y) * g_Player.spd;
		g_Player.pos.z -= cosf(g_Player.rot.y) * g_Player.spd;
	}

#ifdef _DEBUG
	if (GetKeyboardPress(DIK_R))
	{
		g_Player.pos.z = g_Player.pos.x = 0.0f;
		g_Player.spd = 0.0f;
		roty = 0.0f;
	}
#endif

	// プレイヤーの回転
	g_Player.pos.y += sinf(g_Player.rot.x) * cam->len;


	// 弾発射処理
	if (GetKeyboardTrigger(DIK_SPACE))
	{
		SetBullet(g_Player.pos, g_Player.rot);
	}


	// 影もプレイヤーの位置に合わせる
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	SetPositionShadow(g_Player.shadowIdx, pos);


	switch (g_Player.state)
	{
	case IDLE:
		g_Parts[0].tbl_adr = idle_righthand_tbl;
		g_Parts[0].tbl_size = sizeof(idle_righthand_tbl) / sizeof(INTERPOLATION_DATA);
		g_Parts[1].tbl_adr = idle_lefthand_tbl;
		g_Parts[1].tbl_size = sizeof(idle_lefthand_tbl) / sizeof(INTERPOLATION_DATA);
		break;
		
	case MOVE:
		g_Parts[0].tbl_adr = move_righthand_tbl;
		g_Parts[0].tbl_size = sizeof(move_righthand_tbl) / sizeof(INTERPOLATION_DATA);
		g_Parts[1].tbl_adr = move_lefthand_tbl;
		g_Parts[1].tbl_size = sizeof(move_lefthand_tbl) / sizeof(INTERPOLATION_DATA);
		break;
		
	case RUN:
		break;
		
	case ATTACK:
		break;
	}
	
	// 階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// 使われているなら処理する
		if (g_Parts[i].use == TRUE)
		{
			// 移動処理
			int		index = (int)g_Parts[i].move_time;
			float	time = g_Parts[i].move_time - index;
			int		size = g_Parts[i].tbl_size;

			float dt = 1.0f / g_Parts[i].tbl_adr[index].frame;	// 1フレームで進める時間
			g_Parts[i].move_time += dt;					// アニメーションの合計時間に足す

			if (index > (size - 2))	// ゴールをオーバーしていたら、最初へ戻す
			{
				g_Parts[i].move_time = 0.0f;
				index = 0;
			}

			// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
			XMVECTOR p1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].pos);	// 次の場所
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].pos);	// 現在の場所
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[i].pos, p0 + vec * time);

			// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
			XMVECTOR r1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].rot);	// 次の角度
			XMVECTOR r0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].rot);	// 現在の角度
			XMVECTOR rot = r1 - r0;
			XMStoreFloat3(&g_Parts[i].rot, r0 + rot * time);

			// scaleを求める S = StartX + (EndX - StartX) * 今の時間
			XMVECTOR s1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].scl);	// 次のScale
			XMVECTOR s0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].scl);	// 現在のScale
			XMVECTOR scl = s1 - s0;
			XMStoreFloat3(&g_Parts[i].scl, s0 + scl * time);

		}
	}
	
	if (g_Player.pos.x >= 2500.0f)
	{
		g_Player.pos.x = 2500.0f;
	}
	if (g_Player.pos.x <= -2500.0f)
	{
		g_Player.pos.x = -2500.0f;
	}
	if (g_Player.pos.z >= 2500.0f)
	{
		g_Player.pos.z = 2500.0f;
	}
	if (g_Player.pos.z <= -2500.0f)
	{
		g_Player.pos.z = -2500.0f;
	}

	// エネミーとの当たり判定
	ENEMY* enemy = GetEnemy();
	XMFLOAT3 hitPos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 hitNormal;
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (RayHitModel(&enemy[i].model, enemy[i].mtxWorld, g_Player.pos, 50.0f, XMFLOAT3(-sinf(g_Player.rot.y), 0, -cosf(g_Player.rot.y)), &hitPos, &hitNormal))
		{
			SetFade(FADE_OUT, MODE_RESULT);
		}
	}

#ifdef _DEBUG
	// デバッグ表示
	PrintDebugProc("Player X:%f Y:%f Z:%f N:%f\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z, Normal.y);
	PrintDebugProc("Hitpoint X:%f Y:%f Z:%f\n", HitPosition.x, HitPosition.y, HitPosition.z);
#endif

}

//=============================================================================
// 描画処理
//=============================================================================
void DrawPlayer(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	// ワールドマトリックスの初期化
	mtxWorld = XMMatrixIdentity();

	// スケールを反映
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// 回転を反映
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// クォータニオンを反映
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// 移動を反映
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ワールドマトリックスの設定
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


	// 縁取りの設定
	SetFuchi(false);



	// 階層アニメーション
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		if (g_Parts[i].parent != NULL)	// 子供だったら親と結合する
		{
			mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
																			// ↑
																			// g_Player.mtxWorldを指している
		}

		XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

		// 使われているなら処理する
		if (g_Parts[i].use == false) continue;

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);


		// モデル描画
		DrawModel(&g_Parts[i].model);

	}

	SetFuchi(0);

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// プレイヤー情報を取得
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

