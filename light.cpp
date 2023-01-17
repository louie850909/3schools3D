//=============================================================================
//
// ���C�g���� [light.cpp]
// Author : �с@���
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "camera.h"
#include "light.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************



//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void UpdateLightViewMatrix(int index, LIGHT light);

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static LIGHT	g_Light[LIGHT_MAX];

static FOG		g_Fog;

static	BOOL	g_FogEnable = true;


//=============================================================================
// ����������
//=============================================================================
void InitLight(void)
{

	//���C�g������
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		g_Light[i].Position  = XMFLOAT3( 0.0f, 0.0f, 0.0f );
		g_Light[i].Direction = XMFLOAT3( 0.0f, -1.0f, 0.0f );
		g_Light[i].Diffuse   = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f );
		g_Light[i].Ambient   = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		g_Light[i].Attenuation = 100.0f;	// ��������
		g_Light[i].Type = LIGHT_TYPE_NONE;	// ���C�g�̃^�C�v
		g_Light[i].Enable = false;			// ON / OFF
		SetLight(i, &g_Light[i]);
	}

	// ���s�����̐ݒ�i���E���Ƃ炷���j
	g_Light[0].Position = XMFLOAT3(1379.0f, 2000.0f, 1826.0f);
	g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// ���̌���
	g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// ���̐F
	g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// ���s����
	g_Light[0].Enable = true;									// ���̃��C�g��ON
	SetLight(0, &g_Light[0]);									// ����Őݒ肵�Ă���



	// �t�H�O�̏������i���̌��ʁj
	g_Fog.FogStart = 1000.0f;									// ���_���炱�̋��������ƃt�H�O��������n�߂�
	g_Fog.FogEnd   = 10000.0f;									// �����܂ŗ����ƃt�H�O�̐F�Ō����Ȃ��Ȃ�
	g_Fog.FogColor = XMFLOAT4( 0.9f, 0.9f, 0.9f, 1.0f );		// �t�H�O�̐F
	SetFog(&g_Fog);
	SetFogEnable(g_FogEnable);		// ���̏ꏊ���`�F�b�N���� shadow

}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateLight(void)
{
	// ���C�g�̍X�V
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		if (g_Light[i].Enable == true)
		{
			UpdateLightViewMatrix(i, g_Light[i]);
		}
	}
}


//=============================================================================
// ���C�g�̐ݒ�
// Type�ɂ���ăZ�b�g���郁���o�[�ϐ����ς���Ă���
//=============================================================================
void SetLightData(int index, LIGHT *light)
{
	SetLight(index, light);
}


LIGHT *GetLightData(int index)
{
	return(&g_Light[index]);
}


//=============================================================================
// �t�H�O�̐ݒ�
//=============================================================================
void SetFogData(FOG *fog)
{
	SetFog(fog);
}


BOOL	GetFogEnable(void)
{
	return(g_FogEnable);
}

void UpdateLightViewMatrix(int index, LIGHT light)
{
	XMMATRIX LightView = XMMatrixIdentity();
	XMMATRIX LightProjection = XMMatrixIdentity();

	LIGHT_MATRIX lightMatrix;

	switch (light.Type)
	{
	case LIGHT_TYPE_DIRECTIONAL:
		// ���s�����̏ꍇ�́A�J�����̈ʒu�������̈ʒu�Ƃ���
		XMVECTOR eyePos = XMLoadFloat3(&light.Position);
		XMVECTOR lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightView = XMMatrixLookAtLH(eyePos, lookAt, up);
		LightProjection = XMMatrixOrthographicLH(10000.0f, 10000.0f, VIEW_NEAR_Z, VIEW_FAR_Z * 2);

		lightMatrix.LightView = LightView;
		lightMatrix.LightProjection = LightProjection;
		SetLightMatrix(index, &lightMatrix);
		break;

	case LIGHT_TYPE_POINT:
		// �_�����̏ꍇ�́A�J�����̈ʒu�������̈ʒu�Ƃ���
		eyePos = XMLoadFloat3(&light.Position);
		lookAt = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		LightView = XMMatrixLookAtLH(eyePos, lookAt, up);
		LightProjection = XMMatrixPerspectiveFovLH(VIEW_ANGLE, VIEW_ASPECT, VIEW_NEAR_Z, VIEW_FAR_Z);

		lightMatrix.LightView = LightView;
		lightMatrix.LightProjection = LightProjection;
		SetLightMatrix(index, &lightMatrix);
		break;
	}
}
