//=============================================================================
//
// エネミーモデル処理 [enemy.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "input.h"
#include "model.h"
#include "enemy.h"
#include "shadow.h"
#include "meshfield.h"
#include "stage.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************
#define	MODEL_ENEMY				"data/MODEL/body.obj"		// 読み込むモデル名
#define MODEL_ENEMY_LEFTARM		"data/MODEL/left_arm.obj"	
#define MODEL_ENEMY_RIGHTARM	"data/MODEL/right_arm.obj"
#define MODEL_ENEMY_LEFTLEG		"data/MODEL/left_leg.obj"
#define MODEL_ENEMY_RIGHTLEG	"data/MODEL/right_leg.obj"

#define	VALUE_MOVE			(5.0f)						// 移動量
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// 回転量

#define ENEMY_SHADOW_SIZE	(0.4f)						// 影の大きさ
#define ENEMY_OFFSET_Y		(60.0f)						// エネミーの足元をあわせる

#define ENEMY_PARTS_MAX		(4)							// エネミーのパーツの数

#define ENEMY_POS_MIN (-2500.0f)
#define ENEMY_POS_MAX (2500.0f)


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];						// エネミー
static ENEMY			g_Parts[ENEMY_PARTS_MAX * MAX_ENEMY];	// エネミーのパーツ


static int				g_Enemy_load = 0;

static char* g_ModelName[ENEMY_PARTS_MAX] = {	MODEL_ENEMY_LEFTARM,
												MODEL_ENEMY_RIGHTARM,
												MODEL_ENEMY_LEFTLEG,
												MODEL_ENEMY_RIGHTLEG, };

static INTERPOLATION_DATA move_leftarm_tbl[] = {	// pos, rot, scl, frame
	{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 2 },
};

static INTERPOLATION_DATA move_leftleg_tbl[] = {
	{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 2 },
};

static INTERPOLATION_DATA move_rightarm_tbl[] = {
	{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 2 },
};

static INTERPOLATION_DATA move_rightleg_tbl[] = {
	{ XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 60 * 2 },
};

static INTERPOLATION_DATA* g_move_TblAdr[] =
{
	move_leftarm_tbl,
	move_leftleg_tbl,
	move_rightarm_tbl,
	move_rightleg_tbl,
};

static int g_AnimeTblSize[] =
{
	sizeof(move_leftarm_tbl) / sizeof(INTERPOLATION_DATA),
	sizeof(move_rightarm_tbl) / sizeof(INTERPOLATION_DATA),
	sizeof(move_leftleg_tbl) / sizeof(INTERPOLATION_DATA),
	sizeof(move_rightleg_tbl) / sizeof(INTERPOLATION_DATA),
};

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{

		LoadModel(MODEL_ENEMY, &g_Enemy[i].model);
		g_Enemy[i].load = true;

		g_Enemy[i].pos = XMFLOAT3(-550.0f, 200.0f, -2350.0f);
		g_Enemy[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Enemy[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		g_Enemy[i].spd = 0.0f;			// 移動スピードクリア
		g_Enemy[i].size = ENEMY_SIZE;	// 当たり判定の大きさ

		// モデルのディフューズを保存しておく。色変え対応の為。
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// 線形補間用のタイマーをクリア
		g_Enemy[i].tbl_adr = NULL;		// 再生するアニメデータの先頭アドレスをセット
		g_Enemy[i].tbl_size = 0;		// 再生するアニメデータのレコード数をセット

		g_Enemy[i].use = true;			// true:生きてる
		g_Enemy[i].parent = NULL;

		// パーツの初期化
		for (int j = 0; j < ENEMY_PARTS_MAX; j++)
		{
			//char *name = g_ModelName[j];
			LoadModel(g_ModelName[j], &g_Parts[j + i * ENEMY_PARTS_MAX].model);
			g_Parts[j + i * ENEMY_PARTS_MAX].load = 1;
			g_Parts[j + i * ENEMY_PARTS_MAX].use = true;

			g_Parts[j + i * ENEMY_PARTS_MAX].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[j + i * ENEMY_PARTS_MAX].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
			g_Parts[j + i * ENEMY_PARTS_MAX].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

			g_Parts[j + i * ENEMY_PARTS_MAX].parent = &g_Enemy[i];

			g_Parts[j + i * ENEMY_PARTS_MAX].anitbl_no = j;
			g_Parts[j + i * ENEMY_PARTS_MAX].tbl_size = g_AnimeTblSize[j % 4];
			g_Parts[j + i * ENEMY_PARTS_MAX].move_time = 0.0f;
		}
	}


	// 0番だけ線形補間で動かしてみる
	g_Enemy[0].move_time = 0.0f;		// 線形補間用のタイマーをクリア
	//g_Enemy[0].tbl_adr = move_tbl;		// 再生するアニメデータの先頭アドレスをセット
	//g_Enemy[0].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);	// 再生するアニメデータのレコード数をセット

	g_Enemy_load = TRUE;
	return S_OK;
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitEnemy(void)
{
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].load)
		{
			UnloadModel(&g_Enemy[i].model);
			g_Enemy[i].load = false;
		}

		// パーツの終了処理
		for (int j = 0; j < ENEMY_PARTS_MAX; j++)
		{
			if (g_Parts[j + i * ENEMY_PARTS_MAX].load)
			{
				UnloadModel(&g_Parts[j + i * ENEMY_PARTS_MAX].model);
				g_Parts[j + i * ENEMY_PARTS_MAX].load = false;
			}
		}
	}

}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateEnemy(void)
{
	// エネミーを動かく場合は、影も合わせて動かす事を忘れないようにね！
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == true)			// このエネミーが使われている？
		{									// Yes
			if (g_Enemy[i].tbl_adr != NULL)	// 線形補間を実行する？
			{								// 線形補間の処理
				// 移動処理
				int		index = (int)g_Enemy[i].move_time;
				float	time = g_Enemy[i].move_time - index;
				int		size = g_Enemy[i].tbl_size;

				float dt = 1.0f / g_Enemy[i].tbl_adr[index].frame;	// 1フレームで進める時間
				g_Enemy[i].move_time += dt;							// アニメーションの合計時間に足す

				if (index > (size - 2))	// ゴールをオーバーしていたら、最初へ戻す
				{
					g_Enemy[i].move_time = 0.0f;
					index = 0;
				}

				// 座標を求める	X = StartX + (EndX - StartX) * 今の時間
				XMVECTOR p1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].pos);	// 次の場所
				XMVECTOR p0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].pos);	// 現在の場所
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Enemy[i].pos, p0 + vec * time);

				// 回転を求める	R = StartX + (EndX - StartX) * 今の時間
				XMVECTOR r1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].rot);	// 次の角度
				XMVECTOR r0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].rot);	// 現在の角度
				XMVECTOR rot = r1 - r0;
				XMStoreFloat3(&g_Enemy[i].rot, r0 + rot * time);

				// scaleを求める S = StartX + (EndX - StartX) * 今の時間
				XMVECTOR s1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].scl);	// 次のScale
				XMVECTOR s0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].scl);	// 現在のScale
				XMVECTOR scl = s1 - s0;
				XMStoreFloat3(&g_Enemy[i].scl, s0 + scl * time);

			}

			// 影もプレイヤーの位置に合わせる
			XMFLOAT3 pos = g_Enemy[i].pos;
			pos.y -= (ENEMY_OFFSET_Y - 0.1f);
			SetPositionShadow(g_Enemy[i].shadowIdx, pos);

			// レイキャストして足元の高さを求める
			XMFLOAT3 Normal;			// ぶつかったポリゴンの法線ベクトル（向き）
			XMFLOAT3 HitPos;			// ぶつかったポリゴンの座標
			STAGE* stage = GetStage();
			bool ans = RayHitModel(&stage->model, stage->mtxWorld, g_Enemy[i].pos, 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &HitPos, &Normal);
			if (ans == true)
			{
				g_Enemy[i].pos.y = HitPos.y + ENEMY_OFFSET_Y;
			}

			// 階層アニメーション
			for (int j = 0; j < ENEMY_PARTS_MAX; j++)
			{
				// 使われているなら処理する
				if ((g_Parts[j + i * ENEMY_PARTS_MAX].use == true) && (g_Parts[j + i * ENEMY_PARTS_MAX].tbl_size > 0))
				{	// 線形補間の処理
					int nowNo = (int)g_Parts[j + i * ENEMY_PARTS_MAX].move_time;			// 整数分であるテーブル番号を取り出している
					int maxNo = g_Parts[j + i * ENEMY_PARTS_MAX].tbl_size;				// 登録テーブル数を数えている
					int nextNo = (nowNo + 1) % maxNo;			// 移動先テーブルの番号を求めている
					INTERPOLATION_DATA* tbl = g_move_TblAdr[g_Parts[j + i * ENEMY_PARTS_MAX].anitbl_no];	// 行動テーブルのアドレスを取得

					XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTORへ変換
					XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTORへ変換
					XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTORへ変換

					XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ移動量を計算している
					XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ回転量を計算している
					XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ拡大率を計算している

					float nowTime = g_Parts[j + i * ENEMY_PARTS_MAX].move_time - nowNo;	// 時間部分である少数を取り出している

					Pos *= nowTime;								// 現在の移動量を計算している
					Rot *= nowTime;								// 現在の回転量を計算している
					Scl *= nowTime;								// 現在の拡大率を計算している

					// 計算して求めた移動量を現在の移動テーブルXYZに足している＝表示座標を求めている
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].pos, nowPos + Pos);

					// 計算して求めた回転量を現在の移動テーブルに足している
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].rot, nowRot + Rot);

					// 計算して求めた拡大率を現在の移動テーブルに足している
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].scl, nowScl + Scl);

					// frameを使て時間経過処理をする
					g_Parts[j + i * ENEMY_PARTS_MAX].move_time += 1.0f / tbl[nowNo].frame;	// 時間を進めている
					if ((int)g_Parts[j + i * ENEMY_PARTS_MAX].move_time >= maxNo)			// 登録テーブル最後まで移動したか？
					{
						g_Parts[j + i * ENEMY_PARTS_MAX].move_time -= maxNo;				// ０番目にリセットしつつも小数部分を引き継いでいる
					}

				}

			}
		}
	}

#ifdef _DEBUG

#endif // _DEBUG


}

//=============================================================================
// 描画処理
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// カリング無効
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ワールドマトリックスの初期化
		mtxWorld = XMMatrixIdentity();

		// スケールを反映
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// 回転を反映
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// 移動を反映
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ワールドマトリックスの設定
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// モデル描画
		DrawModel(&g_Enemy[i].model);

		// パーツの階層アニメーション
		// 大元の親から子供へと順番に描画処理を行っていく必要があります
		// 配列に親から子供への登録をしないといけません
		for (int j = 0; j < ENEMY_PARTS_MAX; j++)
		{
			// 1. パーツ自身のSRTを行う
			// 2. 親か子供かチェックする
			// 3. 子供の場合、親のワールドマトリックスと自分のワールドマトリックスを掛け算する
			// 4. その結果を使ってパーツを描画する

			// ワールドマトリックスの初期化
			mtxWorld = XMMatrixIdentity();

			// スケールを反映
			mtxScl = XMMatrixScaling(g_Parts[j + i * ENEMY_PARTS_MAX].scl.x, g_Parts[j + i * ENEMY_PARTS_MAX].scl.y, g_Parts[j + i * ENEMY_PARTS_MAX].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// 回転を反映
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[j + i * ENEMY_PARTS_MAX].rot.x, g_Parts[j + i * ENEMY_PARTS_MAX].rot.y, g_Parts[j + i * ENEMY_PARTS_MAX].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// 移動を反映
			mtxTranslate = XMMatrixTranslation(g_Parts[j + i * ENEMY_PARTS_MAX].pos.x, g_Parts[j + i * ENEMY_PARTS_MAX].pos.y, g_Parts[j + i * ENEMY_PARTS_MAX].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[j + i * ENEMY_PARTS_MAX].parent != NULL)	// 子供だったら親と結合する
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[j + i * ENEMY_PARTS_MAX].parent->mtxWorld));
				// ↑
				// g_Player.mtxWorldを指している
			}

			XMStoreFloat4x4(&g_Parts[j + i * ENEMY_PARTS_MAX].mtxWorld, mtxWorld);

			// 使われているなら処理する。ここまで処理している理由は他のパーツがこのパーツを参照している可能性があるから。
			if (g_Parts[j + i * ENEMY_PARTS_MAX].use == false) continue;

			// ワールドマトリックスの設定
			SetWorldMatrix(&mtxWorld);


			// モデル描画
			DrawModel(&g_Parts[j + i * ENEMY_PARTS_MAX].model);
		}
	}

	// カリング設定を戻す
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// エネミーの取得
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}
