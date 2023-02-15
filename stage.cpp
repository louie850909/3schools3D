//=============================================================================
//
// �n�`���� [stage.cpp]
// Author : �с@���
//
//=============================================================================
#include "stage.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define	MODEL_SKYBALL		"data/MODEL/landscape.obj"

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************


//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static STAGE g_stage;

//=============================================================================
// ����������
//=============================================================================
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

//=============================================================================
// �I������
//=============================================================================
void UninitStage(void)
{
	// ���f���̉������
	if (g_stage.load)
	{
		UnloadModel(&g_stage.model);
		g_stage.load = false;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateStage(void)
{
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawStage(void)
{
	XMMATRIX mtxWorld = XMLoadFloat4x4(&g_stage.mtxWorld);

	// ���[���h�}�g���b�N�X��ݒ�
	SetWorldMatrix(&mtxWorld);
	
	

	// ���f���̕`��
	DrawModel(&g_stage.model);
}

//=============================================================================
// ���f���̎擾
//=============================================================================
STAGE* GetStage(void)
{
	return &g_stage;
}
