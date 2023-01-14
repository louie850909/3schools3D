#include "stage.h"

#define	MODEL_SKYBALL		"data/MODEL/landscape.obj"

static STAGE g_stage;

HRESULT InitStage(void)
{
	LoadModel(MODEL_SKYBALL, &g_stage.model);
	g_stage.load = true;

	g_stage.pos = { 0.0f, 0.0f, 0.0f };
	g_stage.rot = { 0.0f, 0.0f, 0.0f };
	g_stage.scl = { 1.0f, 1.0f, 1.0f };

	g_stage.use = true;

	XMMATRIX mtxScl, mtxRot, mtxTranslate, mtxWorld;

	// ���[���h�}�g���b�N�X�̏�����
	mtxWorld = XMMatrixIdentity();

	// �X�P�[���𔽉f
	mtxScl = XMMatrixScaling(g_stage.scl.x, g_stage.scl.y, g_stage.scl.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxScl);

	// ��]�𔽉f
	mtxRot = XMMatrixRotationRollPitchYaw(g_stage.rot.x, g_stage.rot.y + XM_PI, g_stage.rot.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxRot);

	// �ʒu�𔽉f
	mtxTranslate = XMMatrixTranslation(g_stage.pos.x, g_stage.pos.y, g_stage.pos.z);
	mtxWorld = XMMatrixMultiply(mtxWorld, mtxTranslate);

	XMStoreFloat4x4(&g_stage.mtxWorld, mtxWorld);

	return S_OK;
}

void UninitStage(void)
{
	// ���f���̉������
	if (g_stage.load)
	{
		UnloadModel(&g_stage.model);
		g_stage.load = false;
	}
}

void UpdateStage(void)
{
}

void DrawStage(void)
{
	//// �J�����O����
	//SetCullingMode(CULL_MODE_NONE);

	//SetLightEnable(false);

	// Z��r����
	//SetDepthEnable(false);

	// �t�H�O����
	//SetFogEnable(false);

	XMMATRIX mtxWorld = XMLoadFloat4x4(&g_stage.mtxWorld);

	// ���[���h�}�g���b�N�X��ݒ�
	SetWorldMatrix(&mtxWorld);
	
	

	// ���f���̕`��
	DrawModel(&g_stage.model);
	
	//SetLightEnable(true);

	//// �J�����O�ݒ��߂�
	//SetCullingMode(CULL_MODE_BACK);

	// Z��r�L��
	//SetDepthEnable(true);
}

STAGE* GetStage(void)
{
	return &g_stage;
}
