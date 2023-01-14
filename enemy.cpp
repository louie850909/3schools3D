//=============================================================================
//
// �G�l�~�[���f������ [enemy.cpp]
// Author : �с@���
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
// �}�N����`
//*****************************************************************************
#define	MODEL_ENEMY				"data/MODEL/body.obj"		// �ǂݍ��ރ��f����
#define MODEL_ENEMY_LEFTARM		"data/MODEL/left_arm.obj"	
#define MODEL_ENEMY_RIGHTARM	"data/MODEL/right_arm.obj"
#define MODEL_ENEMY_LEFTLEG		"data/MODEL/left_leg.obj"
#define MODEL_ENEMY_RIGHTLEG	"data/MODEL/right_leg.obj"

#define	VALUE_MOVE			(5.0f)						// �ړ���
#define	VALUE_ROTATE		(XM_PI * 0.02f)				// ��]��

#define ENEMY_SHADOW_SIZE	(0.4f)						// �e�̑傫��
#define ENEMY_OFFSET_Y		(60.0f)						// �G�l�~�[�̑��������킹��

#define ENEMY_PARTS_MAX		(4)							// �G�l�~�[�̃p�[�c�̐�

#define ENEMY_POS_MIN (-2500.0f)
#define ENEMY_POS_MAX (2500.0f)


//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ENEMY			g_Enemy[MAX_ENEMY];						// �G�l�~�[
static ENEMY			g_Parts[ENEMY_PARTS_MAX * MAX_ENEMY];	// �G�l�~�[�̃p�[�c


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
// ����������
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

		g_Enemy[i].spd = 0.0f;			// �ړ��X�s�[�h�N���A
		g_Enemy[i].size = ENEMY_SIZE;	// �����蔻��̑傫��

		// ���f���̃f�B�t���[�Y��ۑ����Ă����B�F�ς��Ή��ׁ̈B
		GetModelDiffuse(&g_Enemy[0].model, &g_Enemy[0].diffuse[0]);

		XMFLOAT3 pos = g_Enemy[i].pos;
		pos.y -= (ENEMY_OFFSET_Y - 0.1f);
		g_Enemy[i].shadowIdx = CreateShadow(pos, ENEMY_SHADOW_SIZE, ENEMY_SHADOW_SIZE);

		g_Enemy[i].move_time = 0.0f;	// ���`��ԗp�̃^�C�}�[���N���A
		g_Enemy[i].tbl_adr = NULL;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Enemy[i].tbl_size = 0;		// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

		g_Enemy[i].use = true;			// true:�����Ă�
		g_Enemy[i].parent = NULL;

		// �p�[�c�̏�����
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


	// 0�Ԃ������`��Ԃœ������Ă݂�
	g_Enemy[0].move_time = 0.0f;		// ���`��ԗp�̃^�C�}�[���N���A
	//g_Enemy[0].tbl_adr = move_tbl;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	//g_Enemy[0].tbl_size = sizeof(move_tbl) / sizeof(INTERPOLATION_DATA);	// �Đ�����A�j���f�[�^�̃��R�[�h�����Z�b�g

	g_Enemy_load = TRUE;
	return S_OK;
}

//=============================================================================
// �I������
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

		// �p�[�c�̏I������
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
// �X�V����
//=============================================================================
void UpdateEnemy(void)
{
	// �G�l�~�[�𓮂����ꍇ�́A�e�����킹�ē���������Y��Ȃ��悤�ɂˁI
	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == true)			// ���̃G�l�~�[���g���Ă���H
		{									// Yes
			if (g_Enemy[i].tbl_adr != NULL)	// ���`��Ԃ����s����H
			{								// ���`��Ԃ̏���
				// �ړ�����
				int		index = (int)g_Enemy[i].move_time;
				float	time = g_Enemy[i].move_time - index;
				int		size = g_Enemy[i].tbl_size;

				float dt = 1.0f / g_Enemy[i].tbl_adr[index].frame;	// 1�t���[���Ői�߂鎞��
				g_Enemy[i].move_time += dt;							// �A�j���[�V�����̍��v���Ԃɑ���

				if (index > (size - 2))	// �S�[�����I�[�o�[���Ă�����A�ŏ��֖߂�
				{
					g_Enemy[i].move_time = 0.0f;
					index = 0;
				}

				// ���W�����߂�	X = StartX + (EndX - StartX) * ���̎���
				XMVECTOR p1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].pos);	// ���̏ꏊ
				XMVECTOR p0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].pos);	// ���݂̏ꏊ
				XMVECTOR vec = p1 - p0;
				XMStoreFloat3(&g_Enemy[i].pos, p0 + vec * time);

				// ��]�����߂�	R = StartX + (EndX - StartX) * ���̎���
				XMVECTOR r1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].rot);	// ���̊p�x
				XMVECTOR r0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].rot);	// ���݂̊p�x
				XMVECTOR rot = r1 - r0;
				XMStoreFloat3(&g_Enemy[i].rot, r0 + rot * time);

				// scale�����߂� S = StartX + (EndX - StartX) * ���̎���
				XMVECTOR s1 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 1].scl);	// ����Scale
				XMVECTOR s0 = XMLoadFloat3(&g_Enemy[i].tbl_adr[index + 0].scl);	// ���݂�Scale
				XMVECTOR scl = s1 - s0;
				XMStoreFloat3(&g_Enemy[i].scl, s0 + scl * time);

			}

			// �e���v���C���[�̈ʒu�ɍ��킹��
			XMFLOAT3 pos = g_Enemy[i].pos;
			pos.y -= (ENEMY_OFFSET_Y - 0.1f);
			SetPositionShadow(g_Enemy[i].shadowIdx, pos);

			// ���C�L���X�g���đ����̍��������߂�
			XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
			XMFLOAT3 HitPos;			// �Ԃ������|���S���̍��W
			STAGE* stage = GetStage();
			bool ans = RayHitModel(&stage->model, stage->mtxWorld, g_Enemy[i].pos, 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &HitPos, &Normal);
			if (ans == true)
			{
				g_Enemy[i].pos.y = HitPos.y + ENEMY_OFFSET_Y;
			}

			// �K�w�A�j���[�V����
			for (int j = 0; j < ENEMY_PARTS_MAX; j++)
			{
				// �g���Ă���Ȃ珈������
				if ((g_Parts[j + i * ENEMY_PARTS_MAX].use == true) && (g_Parts[j + i * ENEMY_PARTS_MAX].tbl_size > 0))
				{	// ���`��Ԃ̏���
					int nowNo = (int)g_Parts[j + i * ENEMY_PARTS_MAX].move_time;			// �������ł���e�[�u���ԍ������o���Ă���
					int maxNo = g_Parts[j + i * ENEMY_PARTS_MAX].tbl_size;				// �o�^�e�[�u�����𐔂��Ă���
					int nextNo = (nowNo + 1) % maxNo;			// �ړ���e�[�u���̔ԍ������߂Ă���
					INTERPOLATION_DATA* tbl = g_move_TblAdr[g_Parts[j + i * ENEMY_PARTS_MAX].anitbl_no];	// �s���e�[�u���̃A�h���X���擾

					XMVECTOR nowPos = XMLoadFloat3(&tbl[nowNo].pos);	// XMVECTOR�֕ϊ�
					XMVECTOR nowRot = XMLoadFloat3(&tbl[nowNo].rot);	// XMVECTOR�֕ϊ�
					XMVECTOR nowScl = XMLoadFloat3(&tbl[nowNo].scl);	// XMVECTOR�֕ϊ�

					XMVECTOR Pos = XMLoadFloat3(&tbl[nextNo].pos) - nowPos;	// XYZ�ړ��ʂ��v�Z���Ă���
					XMVECTOR Rot = XMLoadFloat3(&tbl[nextNo].rot) - nowRot;	// XYZ��]�ʂ��v�Z���Ă���
					XMVECTOR Scl = XMLoadFloat3(&tbl[nextNo].scl) - nowScl;	// XYZ�g�嗦���v�Z���Ă���

					float nowTime = g_Parts[j + i * ENEMY_PARTS_MAX].move_time - nowNo;	// ���ԕ����ł��鏭�������o���Ă���

					Pos *= nowTime;								// ���݂̈ړ��ʂ��v�Z���Ă���
					Rot *= nowTime;								// ���݂̉�]�ʂ��v�Z���Ă���
					Scl *= nowTime;								// ���݂̊g�嗦���v�Z���Ă���

					// �v�Z���ċ��߂��ړ��ʂ����݂̈ړ��e�[�u��XYZ�ɑ����Ă��遁�\�����W�����߂Ă���
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].pos, nowPos + Pos);

					// �v�Z���ċ��߂���]�ʂ����݂̈ړ��e�[�u���ɑ����Ă���
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].rot, nowRot + Rot);

					// �v�Z���ċ��߂��g�嗦�����݂̈ړ��e�[�u���ɑ����Ă���
					XMStoreFloat3(&g_Parts[j + i * ENEMY_PARTS_MAX].scl, nowScl + Scl);

					// frame���g�Ď��Ԍo�ߏ���������
					g_Parts[j + i * ENEMY_PARTS_MAX].move_time += 1.0f / tbl[nowNo].frame;	// ���Ԃ�i�߂Ă���
					if ((int)g_Parts[j + i * ENEMY_PARTS_MAX].move_time >= maxNo)			// �o�^�e�[�u���Ō�܂ňړ��������H
					{
						g_Parts[j + i * ENEMY_PARTS_MAX].move_time -= maxNo;				// �O�ԖڂɃ��Z�b�g�������������������p���ł���
					}

				}

			}
		}
	}

#ifdef _DEBUG

#endif // _DEBUG


}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawEnemy(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	for (int i = 0; i < MAX_ENEMY; i++)
	{
		if (g_Enemy[i].use == false) continue;

		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Enemy[i].scl.x, g_Enemy[i].scl.y, g_Enemy[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Enemy[i].rot.x, g_Enemy[i].rot.y + XM_PI, g_Enemy[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Enemy[i].pos.x, g_Enemy[i].pos.y, g_Enemy[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);

		XMStoreFloat4x4(&g_Enemy[i].mtxWorld, mtxWorld);


		// ���f���`��
		DrawModel(&g_Enemy[i].model);

		// �p�[�c�̊K�w�A�j���[�V����
		// �匳�̐e����q���ւƏ��Ԃɕ`�揈�����s���Ă����K�v������܂�
		// �z��ɐe����q���ւ̓o�^�����Ȃ��Ƃ����܂���
		for (int j = 0; j < ENEMY_PARTS_MAX; j++)
		{
			// 1. �p�[�c���g��SRT���s��
			// 2. �e���q�����`�F�b�N����
			// 3. �q���̏ꍇ�A�e�̃��[���h�}�g���b�N�X�Ǝ����̃��[���h�}�g���b�N�X���|���Z����
			// 4. ���̌��ʂ��g���ăp�[�c��`�悷��

			// ���[���h�}�g���b�N�X�̏�����
			mtxWorld = XMMatrixIdentity();

			// �X�P�[���𔽉f
			mtxScl = XMMatrixScaling(g_Parts[j + i * ENEMY_PARTS_MAX].scl.x, g_Parts[j + i * ENEMY_PARTS_MAX].scl.y, g_Parts[j + i * ENEMY_PARTS_MAX].scl.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

			// ��]�𔽉f
			mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[j + i * ENEMY_PARTS_MAX].rot.x, g_Parts[j + i * ENEMY_PARTS_MAX].rot.y, g_Parts[j + i * ENEMY_PARTS_MAX].rot.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

			// �ړ��𔽉f
			mtxTranslate = XMMatrixTranslation(g_Parts[j + i * ENEMY_PARTS_MAX].pos.x, g_Parts[j + i * ENEMY_PARTS_MAX].pos.y, g_Parts[j + i * ENEMY_PARTS_MAX].pos.z);
			mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

			if (g_Parts[j + i * ENEMY_PARTS_MAX].parent != NULL)	// �q����������e�ƌ�������
			{
				mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[j + i * ENEMY_PARTS_MAX].parent->mtxWorld));
				// ��
				// g_Player.mtxWorld���w���Ă���
			}

			XMStoreFloat4x4(&g_Parts[j + i * ENEMY_PARTS_MAX].mtxWorld, mtxWorld);

			// �g���Ă���Ȃ珈������B�����܂ŏ������Ă��闝�R�͑��̃p�[�c�����̃p�[�c���Q�Ƃ��Ă���\�������邩��B
			if (g_Parts[j + i * ENEMY_PARTS_MAX].use == false) continue;

			// ���[���h�}�g���b�N�X�̐ݒ�
			SetWorldMatrix(&mtxWorld);


			// ���f���`��
			DrawModel(&g_Parts[j + i * ENEMY_PARTS_MAX].model);
		}
	}

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}

//=============================================================================
// �G�l�~�[�̎擾
//=============================================================================
ENEMY *GetEnemy()
{
	return &g_Enemy[0];
}
