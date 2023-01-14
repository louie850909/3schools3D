#include "skyball.h"

#define	MODEL_SKYBALL		"data/MODEL/skyball.obj"

static SKYBALL g_skyball;

HRESULT InitSkyBall(void)
{
	LoadModel(MODEL_SKYBALL, &g_skyball.model);
	g_skyball.load = true;

	g_skyball.pos = { 1.0f, 1.0f, 1.0f };
	g_skyball.rot = { 0.0f, 0.0f, 0.0f };
	g_skyball.scl = { 0.01f, 0.01f, 0.01f };

	g_skyball.use = true;

	return S_OK;
}

void UninitSkyBall(void)
{
	// ���f���̉������
	if (g_skyball.load)
	{
		UnloadModel(&g_skyball.model);
		g_skyball.load = false;
	}
}

void UpdateSkyBall(void)
{
}

void DrawSkyBall(void)
{
	// �J�����O����
	SetCullingMode(CULL_MODE_NONE);

	SetLightEnable(false);

	// Z��r����
	SetDepthEnable(false);

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


	// ���f���`��
	DrawModel(&g_skyball.model);
	//DrawInstanceModel(&g_Player.model, 1, g_InstancePlayer);

	SetLightEnable(true);

	// �J�����O�ݒ��߂�
	SetCullingMode(CULL_MODE_BACK);

	// Z��r�L��
	SetDepthEnable(true);

	// �t�H�O�L��
	//SetFogEnable(true);
}
