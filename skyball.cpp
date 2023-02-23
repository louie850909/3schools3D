//=============================================================================
//
// �󏈗� [skyball.cpp]
// Author : �с@���
//
//=============================================================================
#include "skyball.h"
#include "player.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_SKYBALL		"data/MODEL/skyball.obj"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static SKYBALL g_skyball;
static float g_playerdistX;
static float g_playerdistY;
static float g_playerdistZ;

//=============================================================================
// ����������
//=============================================================================
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

//=============================================================================
// �I������
//=============================================================================
void UninitSkyBall(void)
{
	// ���f���̉������
	if (g_skyball.load)
	{
		UnloadModel(&g_skyball.model);
		g_skyball.load = false;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateSkyBall(void)
{
	PLAYER* player = GetPlayer();
	g_skyball.pos.x = player->pos.x + g_playerdistX;
	g_skyball.pos.y = player->pos.y + g_playerdistY;
	g_skyball.pos.z = player->pos.z + g_playerdistZ;
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawSkyBall(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	SetLightEnable(false);

	// Z��r����
	SetDepthEnable(false);

	SetAlphaTestEnable(false);

	// �t�H�O����
	//SetFogEnable(false);

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_skyball.scl.x, g_skyball.scl.y, g_skyball.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_skyball.rot.x, g_skyball.rot.y + XM_PI, g_skyball.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ړ��𔽉f
	mtxTranslate = XMMatrixTranslation(g_skyball.pos.x, g_skyball.pos.y, g_skyball.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	// ���[���h�}�g���b�N�X�̐ݒ�
	SetWorldMatrix(&mtxWorld);

	XMStoreFloat4x4(&g_skyball.mtxWorld, mtxWorld);

	
	SetShaderMode(SHADER_MODE_SKY);

	// ���f���`��
	DrawModel(&g_skyball.model);
	
	SetAlphaTestEnable(false);
	
	SetShaderMode(SHADER_MODE_DEFAULT);

	SetLightEnable(true);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);

	// Z��r�L��
	SetDepthEnable(true);

	// �t�H�O�L��
	//SetFogEnable(true);
}
