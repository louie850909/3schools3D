//=============================================================================
//
// �؏��� [tree.cpp]
// Author : �с@���
//
//=============================================================================
#include "main.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "shadow.h"
#include "tree.h"
#include "model.h"
#include "stage.h"
#include "file.h"
#include <thread>


//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TYPE_MAX			(3)				// �؂̎�ނ̍ő吔

#define TREE_POS_MAX		(2500.0f)			// �����ʒu�ő�l
#define TREE_POS_MIN		(-2500.0f)			// �����ʒu�ŏ��l

#define	MODEL_TREE_01		"data/MODEL/tree.obj"
#define	MODEL_TREE_02		"data/MODEL/tree2.obj"
#define	MODEL_TREE_03		"data/MODEL/tree3.obj"

//*****************************************************************************
// �\���̒�`
//*****************************************************************************
typedef struct
{	
	bool	    load; 		 // ���f���ǂݍ��݃t���O
	DX11_MODEL	model;		 // ���f�����
	
	bool		use;		 // �g�p�t���O
} TREE;

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
void SetTreeHeightA();
void SetTreeHeightB();
void SetTreeHeightC();
void SetTreeHeightD();
void SetTreeHeightE();
void SetTreeHeightF();
void SetTreeHeightG();
void SetTreeHeightH();

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer					*g_VertexBuffer = NULL;	// ���_�o�b�t�@
static ID3D11Buffer					*g_InstanceBuffer = NULL; // �C���X�^���X�o�b�t�@

static TREE					g_aTree[TYPE_MAX];	// �؂̎��
static int					g_TypeNo;			// ��ޔԍ�
static bool					g_bAlpaTest;		// �A���t�@�e�X�gON/OFF

static INSTANCE g_aInstance[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance2[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance3[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance4[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance5[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance6[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance7[MAX_TREE / 8];			// �C���X�^���X�f�[�^
static INSTANCE g_aInstance8[MAX_TREE / 8];			// �C���X�^���X�f�[�^

static INSTANCE g_totalInstanceTree[MAX_TREE];

static char *g_ModelName[] =
{
	MODEL_TREE_01,
	MODEL_TREE_02,
	MODEL_TREE_03,
};

//=============================================================================
// ����������
//=============================================================================
HRESULT InitTree(void)
{
	STAGE* stage = GetStage();
	
	for (int i = 0; i < TYPE_MAX; i++)
	{
		LoadModel(g_ModelName[i], &g_aTree[i].model);
		g_aTree[i].load = true;
		g_aTree[i].use = false;
	}
	
	// �C���X�^���X�o�b�t�@����
	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(instanceBufferDesc));
	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(INSTANCE) * MAX_TREE;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_MAPPED_SUBRESOURCE instanceData;

	GetDevice()->CreateBuffer(&instanceBufferDesc, NULL, &g_InstanceBuffer);

	GetDeviceContext()->Map(g_InstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &instanceData);

	INSTANCE* instance = (INSTANCE*)instanceData.pData;

	// BIN�f�[�^����؂̈ʒu��ǂݍ��ށA�Ȃ��Ȃ琶������
	FILE* fp = fopen("data/BIN/tree.bin", "rb");
	if (fp != NULL)
	{
		fclose(fp);
		LoadData(SAVE_TREE);

		for (int i = 0; i < MAX_TREE; i++)
		{
			instance[i] = g_totalInstanceTree[i];
		}
	}
	else
	{
		std::thread th1(SetTreeHeightA);
		std::thread th2(SetTreeHeightB);
		std::thread th3(SetTreeHeightC);
		std::thread th4(SetTreeHeightD);
		std::thread th5(SetTreeHeightE);
		std::thread th6(SetTreeHeightF);
		std::thread th7(SetTreeHeightG);
		std::thread th8(SetTreeHeightH);

		th1.join();
		th2.join();
		th3.join();
		th4.join();
		th5.join();
		th6.join();
		th7.join();
		th8.join();

		for (int i = 0; i < MAX_TREE; i++)
		{
			if (i < MAX_TREE / 8)
			{
				instance[i] = g_aInstance[i];
				g_totalInstanceTree[i] = g_aInstance[i];
			}
			else if (i < MAX_TREE / 4)
			{
				instance[i] = g_aInstance2[i - MAX_TREE / 8];
				g_totalInstanceTree[i] = g_aInstance2[i - MAX_TREE / 8];
			}
			else if (i < MAX_TREE * 3 / 8)
			{
				instance[i] = g_aInstance3[i - MAX_TREE / 4];
				g_totalInstanceTree[i] = g_aInstance3[i - MAX_TREE / 4];
			}
			else if (i < MAX_TREE / 2)
			{
				instance[i] = g_aInstance4[i - MAX_TREE * 3 / 8];
				g_totalInstanceTree[i] = g_aInstance4[i - MAX_TREE * 3 / 8];
			}
			else if (i < MAX_TREE * 5 / 8)
			{
				instance[i] = g_aInstance5[i - MAX_TREE / 2];
				g_totalInstanceTree[i] = g_aInstance5[i - MAX_TREE / 2];
			}
			else if (i < MAX_TREE * 3 / 4)
			{
				instance[i] = g_aInstance6[i - MAX_TREE * 5 / 8];
				g_totalInstanceTree[i] = g_aInstance6[i - MAX_TREE * 5 / 8];
			}
			else if (i < MAX_TREE * 7 / 8)
			{
				instance[i] = g_aInstance7[i - MAX_TREE * 3 / 4];
				g_totalInstanceTree[i] = g_aInstance7[i - MAX_TREE * 3 / 4];
			}
			else
			{
				instance[i] = g_aInstance8[i - MAX_TREE * 7 / 8];
				g_totalInstanceTree[i] = g_aInstance8[i - MAX_TREE * 7 / 8];
			}
		}

		// �o�C�i���t�@�C���ɕۑ�
		SaveData(SAVE_TREE);
	}
	GetDeviceContext()->Unmap(g_InstanceBuffer, 0);

	g_bAlpaTest = true;

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitTree(void)
{
	for(int nCntTex = 0; nCntTex < TYPE_MAX; nCntTex++)
	{
		if (g_aTree[nCntTex].load == true)
		{
			UnloadModel(&g_aTree[nCntTex].model);
			g_aTree[nCntTex].load = false;
		}
	}

	if(g_VertexBuffer != NULL)
	{// ���_�o�b�t�@�̉��
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	if (g_InstanceBuffer != NULL)
	{// �C���X�^���X�o�b�t�@�̉��
		g_InstanceBuffer->Release();
		g_InstanceBuffer = NULL;
	}
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateTree(void)
{
#ifdef _DEBUG
	// �A���t�@�e�X�gON/OFF
	if(GetKeyboardTrigger(DIK_F1))
	{
		g_bAlpaTest = g_bAlpaTest ? false: true;
	}
#endif

}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawTree(void)
{
	// ���e�X�g�ݒ�
	if (g_bAlpaTest == true)
	{
		// ���e�X�g��L����
		SetAlphaTestEnable(true);
	}

	// ���C�e�B���O�𖳌�
	//SetLightEnable(false);

	DrawInstanceModel(&g_aTree[g_TypeNo].model, MAX_TREE, g_InstanceBuffer, SHADER_MODE_INSTANCING);

	// ���C�e�B���O��L����
	//SetLightEnable(true);

	// ���e�X�g�𖳌���
	SetAlphaTestEnable(false);
}

void DrawTreeShadowMap(void)
{
	DrawInstanceModel(&g_aTree[g_TypeNo].model, MAX_TREE, g_InstanceBuffer, SHADER_MODE_SHADOW_MAP_INST);
}

INSTANCE* getTreeInstance()
{
	return &g_totalInstanceTree[0];
}

void SetTreeHeightA()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance[i].pos.x, 1000.0f, g_aInstance[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance[i].pos.y = 20.0f;
		}
		g_aInstance[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightB()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance2[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance2[i].pos.x, 1000.0f, g_aInstance2[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance2[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance2[i].pos.y = 20.0f;
		}
		g_aInstance2[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance2[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightC()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance3[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance3[i].pos.x, 1000.0f, g_aInstance3[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance3[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance3[i].pos.y = 20.0f;
		}
		g_aInstance3[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance3[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightD()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance4[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance4[i].pos.x, 1000.0f, g_aInstance4[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance4[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance4[i].pos.y = 20.0f;
		}
		g_aInstance4[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance4[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightE()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance5[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance5[i].pos.x, 1000.0f, g_aInstance5[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance5[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance5[i].pos.y = 20.0f;
		}
		g_aInstance5[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance5[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightF()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance6[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance6[i].pos.x, 1000.0f, g_aInstance6[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance6[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance6[i].pos.y = 20.0f;
		}
		g_aInstance6[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance6[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightG()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance7[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance7[i].pos.x, 1000.0f, g_aInstance7[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance7[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance7[i].pos.y = 20.0f;
		}
		g_aInstance7[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance7[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}

void SetTreeHeightH()
{
	srand((unsigned int)clock() + _threadid);
	STAGE* stage = GetStage();
	XMFLOAT3 hitPos;
	XMFLOAT3 hitNormal;

	for (int i = 0; i < MAX_TREE / 8; i++)
	{
		g_aInstance8[i].pos = XMFLOAT4((float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 20.0f, (float)(TREE_POS_MIN + rand() % (int)(TREE_POS_MAX - TREE_POS_MIN)), 0.0f);
		bool ans = RayHitModel(&stage->model, stage->mtxWorld, XMFLOAT3(g_aInstance8[i].pos.x, 1000.0f, g_aInstance8[i].pos.z), 2000.0f, XMFLOAT3(0.0f, -1.0f, 0.0f), &hitPos, &hitNormal);

		if (ans == true)
		{
			g_aInstance8[i].pos.y = hitPos.y;
		}
		else
		{
			g_aInstance8[i].pos.y = 20.0f;
		}
		g_aInstance8[i].scl = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		g_aInstance8[i].rot = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
}
