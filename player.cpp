//=============================================================================
//
// ���f������ [player.cpp]
// Author : �с@���
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
// �}�N����`
//*****************************************************************************
#define	MODEL_PLAYER			"data/MODEL/cone.obj"				// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_LEFT_HAND	"data/MODEL/human_left_hand.obj"	// �ǂݍ��ރ��f����
#define	MODEL_PLAYER_RIGHT_HAND	"data/MODEL/hand_and_sword.obj"		// �ǂݍ��ރ��f����

#define	VALUE_MOVE			(2.0f)							// �ړ���
#define	VALUE_ROTATE		(D3DX_PI * 0.02f)				// ��]��

#define PLAYER_SHADOW_SIZE	(0.4f)							// �e�̑傫��
#define PLAYER_OFFSET_Y		(30.0f)							// �v���C���[�̑��������킹��

#define PLAYER_PARTS_MAX	(2)								// �v���C���[�̃p�[�c�̐�

#define INIT_POS_X			(1095.0f)							// �����ʒu(X���W)
#define INIT_POS_Y			(680.0f)							// �����ʒu(Y���W)
#define INIT_POS_Z			(1054.0f)							// �����ʒu(Z���W)



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static PLAYER		g_Player;						// �v���C���[

static PLAYER		g_Parts[PLAYER_PARTS_MAX];		// �v���C���[�̃p�[�c�p

static float		roty = 0.0f;

static LIGHT		g_Light;

// �v���C���[�̊K�w�A�j���[�V�����f�[�^
// �v���C���[�̕����Ă���A�j���f�[�^
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


// �v���C���[�̑����Ă���A�j���f�[�^
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
// ����������
//=============================================================================
HRESULT InitPlayer(void)
{
	g_Player.load = TRUE;
	LoadModel(MODEL_PLAYER, &g_Player.model);

	g_Player.pos = XMFLOAT3(INIT_POS_X, PLAYER_OFFSET_Y+INIT_POS_Y, INIT_POS_Z);
	g_Player.rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player.state = MOVE;

	g_Player.spd = 0.0f;			// �ړ��X�s�[�h�N���A

	g_Player.use = TRUE;			// true:�����Ă�
	g_Player.size = PLAYER_SIZE;	// �����蔻��̑傫��

	// �����Ńv���C���[�p�̉e���쐬���Ă���
	XMFLOAT3 pos = g_Player.pos;
	pos.y -= (PLAYER_OFFSET_Y - 0.1f);
	g_Player.shadowIdx = CreateShadow(pos, PLAYER_SHADOW_SIZE, PLAYER_SHADOW_SIZE);
	//          ��
	//        ���̃����o�[�ϐ������������e��Index�ԍ�

	// �L�[�����������̃v���C���[�̌���
	roty = 0.0f;

	g_Player.parent = NULL;			// �{�́i�e�j�Ȃ̂�NULL������


	// �K�w�A�j���[�V�����̏�����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		g_Parts[i].use = TRUE;

		// �ʒu�E��]�E�X�P�[���̏����ݒ�
		g_Parts[i].pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
		g_Parts[i].scl = XMFLOAT3(1.0f, 1.0f, 1.0f);

		// �e�q�֌W
		g_Parts[i].parent = &g_Player;		// �� �����ɐe�̃A�h���X������
	//	g_Parts[�r].parent= &g_Player;		// �r��������e�͖{�́i�v���C���[�j
	//	g_Parts[��].parent= &g_Paerts[�r];	// �w���r�̎q���������ꍇ�̗�

		// �K�w�A�j���[�V�����p�̃����o�[�ϐ��̏�����
		//g_Parts[i].tbl_adr = move_tbl;	// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
		g_Parts[i].move_time = 0.0f;	// ���s���Ԃ��N���A
	}
	// �p�[�c�̓ǂݍ���
	g_Parts[0].load = TRUE;
	LoadModel(MODEL_PLAYER_RIGHT_HAND, &g_Parts[0].model);
	g_Parts[0].parent = &g_Player;		// 
	g_Parts[0].tbl_adr = move_righthand_tbl;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g
	
	// �p�[�c�̓ǂݍ���
	g_Parts[1].load = TRUE;
	LoadModel(MODEL_PLAYER_LEFT_HAND, &g_Parts[1].model);
	g_Parts[1].parent = &g_Player;		// 
	g_Parts[1].tbl_adr = move_lefthand_tbl;		// �Đ�����A�j���f�[�^�̐擪�A�h���X���Z�b�g


	// �N�H�[�^�j�I���̏�����
	XMStoreFloat4(&g_Player.Quaternion, XMQuaternionIdentity());



	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitPlayer(void)
{
	// ���f���̉������
	if (g_Player.load == TRUE)
	{
		UnloadModel(&g_Player.model);
		g_Player.load = FALSE;
	}

	// �p�[�c�̉������
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		if (g_Parts[i].load == TRUE)
		{
			// �p�[�c�̉������
			UnloadModel(&g_Parts[i].model);
			g_Parts[i].load = FALSE;
		}
	}

}

//=============================================================================
// �X�V����
//=============================================================================
void UpdatePlayer(void)
{
	g_Player.state = IDLE;
	XMFLOAT3 prePos = g_Player.pos;		// �O��̍��W��ۑ�
	
	CAMERA *cam = GetCamera();

	g_Player.spd *= 0.9f;
	g_Player.rot.y = cam->rot.y;
	g_Player.rot.x = cam->rot.x;

	// ���C�L���X�g���đ����̍��������߂�
	XMFLOAT3 HitPosition;		// ��_
	XMFLOAT3 Normal;			// �Ԃ������|���S���̖@���x�N�g���i�����j
	STAGE* stage = GetStage();
	bool ans = RayHitModel(&stage->model, stage->mtxWorld, g_Player.pos, 300.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &HitPosition, &Normal);
	if (ans)
	{
		g_Player.pos.y = HitPosition.y + PLAYER_OFFSET_Y;
	}

	// fps�ړ�����
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

		// ����̎��ɂ�������U��������
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

		// ����̎��ɂ�������U��������
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

	// �v���C���[�̉�]
	g_Player.pos.y += sinf(g_Player.rot.x) * cam->len;


	// �e���ˏ���
	if (GetKeyboardTrigger(DIK_SPACE))
	{
		SetBullet(g_Player.pos, g_Player.rot);
	}


	// �e���v���C���[�̈ʒu�ɍ��킹��
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
	
	// �K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// �g���Ă���Ȃ珈������
		if (g_Parts[i].use == TRUE)
		{
			// �ړ�����
			int		index = (int)g_Parts[i].move_time;
			float	time = g_Parts[i].move_time - index;
			int		size = g_Parts[i].tbl_size;

			float dt = 1.0f / g_Parts[i].tbl_adr[index].frame;	// 1�t���[���Ői�߂鎞��
			g_Parts[i].move_time += dt;					// �A�j���[�V�����̍��v���Ԃɑ���

			if (index > (size - 2))	// �S�[�����I�[�o�[���Ă�����A�ŏ��֖߂�
			{
				g_Parts[i].move_time = 0.0f;
				index = 0;
			}

			// ���W�����߂�	X = StartX + (EndX - StartX) * ���̎���
			XMVECTOR p1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].pos);	// ���̏ꏊ
			XMVECTOR p0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].pos);	// ���݂̏ꏊ
			XMVECTOR vec = p1 - p0;
			XMStoreFloat3(&g_Parts[i].pos, p0 + vec * time);

			// ��]�����߂�	R = StartX + (EndX - StartX) * ���̎���
			XMVECTOR r1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].rot);	// ���̊p�x
			XMVECTOR r0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].rot);	// ���݂̊p�x
			XMVECTOR rot = r1 - r0;
			XMStoreFloat3(&g_Parts[i].rot, r0 + rot * time);

			// scale�����߂� S = StartX + (EndX - StartX) * ���̎���
			XMVECTOR s1 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 1].scl);	// ����Scale
			XMVECTOR s0 = XMLoadFloat3(&g_Parts[i].tbl_adr[index + 0].scl);	// ���݂�Scale
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

	// �G�l�~�[�Ƃ̓����蔻��
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
	// �f�o�b�O�\��
	PrintDebugProc("Player X:%f Y:%f Z:%f N:%f\n", g_Player.pos.x, g_Player.pos.y, g_Player.pos.z, Normal.y);
	PrintDebugProc("Hitpoint X:%f Y:%f Z:%f\n", HitPosition.x, HitPosition.y, HitPosition.z);
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawPlayer(void)
{
	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld, quatMatrix;

	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_Player.scl.x, g_Player.scl.y, g_Player.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_Player.rot.x, g_Player.rot.y + XM_PI, g_Player.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �N�H�[�^�j�I���𔽉f
	quatMatrix = XMMatrixRotationQuaternion(XMLoadFloat4(&g_Player.Quaternion));
	mtxWorld = XMMatrixMultiply(mtxWorld, quatMatrix);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_Player.pos.x, g_Player.pos.y, g_Player.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_Player.mtxWorld, mtxWorld);


	// �����̐ݒ�
	SetFuchi(false);



	// �K�w�A�j���[�V����
	for (int i = 0; i < PLAYER_PARTS_MAX; i++)
	{
		// ���[���h�}�g���b�N�X�̏�����
		mtxWorld = XMMatrixIdentity();

		// �X�P�[���𔽉f
		mtxScl = XMMatrixScaling(g_Parts[i].scl.x, g_Parts[i].scl.y, g_Parts[i].scl.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

		// ��]�𔽉f
		mtxRot = XMMatrixRotationRollPitchYaw(g_Parts[i].rot.x, g_Parts[i].rot.y, g_Parts[i].rot.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

		// �ړ��𔽉f
		mtxTranslate = XMMatrixTranslation(g_Parts[i].pos.x, g_Parts[i].pos.y, g_Parts[i].pos.z);
		mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

		if (g_Parts[i].parent != NULL)	// �q����������e�ƌ�������
		{
			mtxWorld = XMMatrixMultiply(mtxWorld, XMLoadFloat4x4(&g_Parts[i].parent->mtxWorld));
																			// ��
																			// g_Player.mtxWorld���w���Ă���
		}

		XMStoreFloat4x4(&g_Parts[i].mtxWorld, mtxWorld);

		// �g���Ă���Ȃ珈������
		if (g_Parts[i].use == false) continue;

		// ���[���h�}�g���b�N�X�̐ݒ�
		SetWorldMatrix(&mtxWorld);


		// ���f���`��
		DrawModel(&g_Parts[i].model);

	}

	SetFuchi(0);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);
}


//=============================================================================
// �v���C���[�����擾
//=============================================================================
PLAYER *GetPlayer(void)
{
	return &g_Player;
}

