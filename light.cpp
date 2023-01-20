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
#define LIGHT_SUNRISE_POS_X		(5000.3f)		// ���̏o��X�ʒu
#define LIGHT_SUNRISE_POS_Y		(-869.3f)		// ���̏o��Y�ʒu
#define LIGHT_SUNRISE_POS_Z		(0.0f)			// ���̏o��Z�ʒu
#define LIGHT_SUNSET_POS_X		(-2245.3f)		// ���̓����X�ʒu
#define LIGHT_SUNSET_POS_Y		(-869.3f)		// ���̓����Y�ʒu
#define LIGHT_SUNSET_POS_Z		(5000.0f)		// ���̓����Z�ʒu
#define LIGHT_NOON_POS_X		(1377.35f)		// ����X�ʒu
#define LIGHT_NOON_POS_Y		(5000.0f)		// ����Y�ʒu
#define LIGHT_NOON_POS_Z		(2500.0f)		// ����Z�ʒu
#define LIGHT_MIDNINGT_POS_X	(1377.35f)		// ���X�ʒu
#define LIGHT_MIDNINGT_POS_Y	(-5000.0f)		// ���Y�ʒu
#define LIGHT_MIDNINGT_POS_Z	(2500.0f)		// ���Z�ʒu

#define SUN_MOVING_TIME			(600.0f)		// ���̈ړ����ԁi�t���[���j
enum TIME
{
	SUNRISE,
	NOON,
	SUNSET,
	MIDNIGHT,

	TIME_NUM,
};
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

static int		g_nowTime = SUNRISE;
static float	g_time = -1;

XMFLOAT3 RiseToNoon = XMFLOAT3(
	(LIGHT_NOON_POS_X - LIGHT_SUNRISE_POS_X) / SUN_MOVING_TIME,
	(LIGHT_NOON_POS_Y - LIGHT_SUNRISE_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_NOON_POS_Z - LIGHT_SUNRISE_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 NoonToSet = XMFLOAT3(
	(LIGHT_SUNSET_POS_X - LIGHT_NOON_POS_X) / SUN_MOVING_TIME,
	(LIGHT_SUNSET_POS_Y - LIGHT_NOON_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_SUNSET_POS_Z - LIGHT_NOON_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 SetToMidnight = XMFLOAT3(
	(LIGHT_MIDNINGT_POS_X - LIGHT_SUNSET_POS_X) / SUN_MOVING_TIME,
	(LIGHT_MIDNINGT_POS_Y - LIGHT_SUNSET_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_MIDNINGT_POS_Z - LIGHT_SUNSET_POS_Z) / SUN_MOVING_TIME
);
XMFLOAT3 MidnightToRise = XMFLOAT3(
	(LIGHT_SUNRISE_POS_X - LIGHT_MIDNINGT_POS_X) / SUN_MOVING_TIME,
	(LIGHT_SUNRISE_POS_Y - LIGHT_MIDNINGT_POS_Y) / SUN_MOVING_TIME,
	(LIGHT_SUNRISE_POS_Z - LIGHT_MIDNINGT_POS_Z) / SUN_MOVING_TIME
);

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

	switch (GetMode())
	{
	case MODE_TITLE:
		g_nowTime = SUNRISE;
		
		// ���s�����̐ݒ�i���E���Ƃ炷���j
		g_Light[0].Position = XMFLOAT3(LIGHT_SUNRISE_POS_X, LIGHT_SUNRISE_POS_Y, LIGHT_SUNRISE_POS_Z);
		g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// ���̌���
		g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// ���̐F
		g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// ���s����
		g_Light[0].Enable = true;									// ���̃��C�g��ON
		SetLight(0, &g_Light[0]);									// ����Őݒ肵�Ă���

		// �t�H�O�̏������i���̌��ʁj
		g_Fog.FogStart = 1000.0f;									// ���_���炱�̋��������ƃt�H�O��������n�߂�
		g_Fog.FogEnd = 10000.0f;									// �����܂ŗ����ƃt�H�O�̐F�Ō����Ȃ��Ȃ�
		g_Fog.FogColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);		// �t�H�O�̐F
		SetFog(&g_Fog);
		SetFogEnable(g_FogEnable);		// ���̏ꏊ���`�F�b�N���� shadow
		break;
		
	case MODE_GAME:
		g_nowTime = NOON;
		
		// ���s�����̐ݒ�i���E���Ƃ炷���j
		g_Light[0].Position = XMFLOAT3(LIGHT_NOON_POS_X, LIGHT_NOON_POS_Y, LIGHT_NOON_POS_Z);
		g_Light[0].Direction = XMFLOAT3(0 - g_Light[0].Position.x, 0 - g_Light[0].Position.y, 0 - g_Light[0].Position.z);		// ���̌���
		g_Light[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);	// ���̐F
		g_Light[0].Type = LIGHT_TYPE_DIRECTIONAL;					// ���s����
		g_Light[0].Enable = true;									// ���̃��C�g��ON
		SetLight(0, &g_Light[0]);									// ����Őݒ肵�Ă���

		// �t�H�O�̏������i���̌��ʁj
		g_Fog.FogStart = 1000.0f;									// ���_���炱�̋��������ƃt�H�O��������n�߂�
		g_Fog.FogEnd = 10000.0f;									// �����܂ŗ����ƃt�H�O�̐F�Ō����Ȃ��Ȃ�
		g_Fog.FogColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);		// �t�H�O�̐F
		SetFog(&g_Fog);
		SetFogEnable(g_FogEnable);		// ���̏ꏊ���`�F�b�N���� shadow
	}
	
	g_time = -1;
}


//=============================================================================
// �X�V����
//=============================================================================
void UpdateLight(void)
{
	g_time++;
	switch (GetMode())
	{
	case MODE_TITLE:
		
		switch (g_nowTime)
		{
		case SUNRISE:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_SUNRISE_POS_X + g_time * RiseToNoon.x;
				g_Light[0].Position.y = LIGHT_SUNRISE_POS_Y + g_time * RiseToNoon.y;
				g_Light[0].Position.z = LIGHT_SUNRISE_POS_Z + g_time * RiseToNoon.z;

				g_Fog.FogColor.x += 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.y += 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.z += 1.0f / SUN_MOVING_TIME;
			}
			else
			{
				g_time = -1;
				g_nowTime = NOON;
			}
			break;

		case NOON:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_NOON_POS_X + g_time * NoonToSet.x;
				g_Light[0].Position.y = LIGHT_NOON_POS_Y + g_time * NoonToSet.y;
				g_Light[0].Position.z = LIGHT_NOON_POS_Z + g_time * NoonToSet.z;

				g_Fog.FogColor.x -= 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.y -= 1.0f / SUN_MOVING_TIME;
				g_Fog.FogColor.z -= 1.0f / SUN_MOVING_TIME;
			}
			else
			{
				g_time = -1;
				g_nowTime = SUNSET;
			}
			break;

		case SUNSET:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_SUNSET_POS_X + g_time * SetToMidnight.x;
				g_Light[0].Position.y = LIGHT_SUNSET_POS_Y + g_time * SetToMidnight.y;
				g_Light[0].Position.z = LIGHT_SUNSET_POS_Z + g_time * SetToMidnight.z;
			}
			else
			{
				g_time = -1;
				g_nowTime = MIDNIGHT;
			}
			break;

		case MIDNIGHT:
			if (g_time < SUN_MOVING_TIME)
			{
				g_Light[0].Position.x = LIGHT_MIDNINGT_POS_X + g_time * MidnightToRise.x;
				g_Light[0].Position.y = LIGHT_MIDNINGT_POS_Y + g_time * MidnightToRise.y;
				g_Light[0].Position.z = LIGHT_MIDNINGT_POS_Z + g_time * MidnightToRise.z;
			}
			else
			{
				g_time = -1;
				g_nowTime = SUNRISE;
			}
			break;
		}
		break;

	case MODE_GAME:
		break;
	}

	// ���C�g�̍X�V
	for (int i = 0; i < LIGHT_MAX; i++)
	{
		if (g_Light[i].Enable == true)
		{
			UpdateLightViewMatrix(i, g_Light[i]);
		}
	}
	SetLightData(0, &g_Light[0]);
	SetFog(&g_Fog);
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
