#pragma once
#include "model.h"

struct SKYBALL
{
	XMFLOAT3		pos;		// �|���S���̈ʒu
	XMFLOAT3		rot;		// �|���S���̌���(��])
	XMFLOAT3		scl;		// �|���S���̑傫��(�X�P�[��)

	XMFLOAT4X4		mtxWorld;	// ���[���h�}�g���b�N�X

	bool			load;
	DX11_MODEL		model;		// ���f�����
	
	bool			use;
};

HRESULT InitSkyBall(void);
void UninitSkyBall(void);
void UpdateSkyBall(void);
void DrawSkyBall(void);