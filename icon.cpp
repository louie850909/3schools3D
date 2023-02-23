//=============================================================================
//
// �~�j�}�b�v�p�A�C�R������ [icon.cpp]
// Author : �с@���
//
//=============================================================================
#include "icon.h"
#include "player.h"
#include "enemy.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
#define TEXTURE_WIDTH				(128)	// �L�����T�C�Y
#define TEXTURE_HEIGHT				(128)	// 
#define TEXTURE_MAX					(2)		// �e�N�X�`���̐�

//*****************************************************************************
// �O���[�o���ϐ�
//*****************************************************************************
static ID3D11Buffer* g_VertexBuffer = NULL;							// ���_���
static ID3D11ShaderResourceView* g_Texture[TEXTURE_MAX] = { NULL };	// �e�N�X�`�����

static char* g_TexturName[TEXTURE_MAX] = {
	"data/TEXTURE/circle.png",
	"data/TEXTURE/cross.png",
};

static ICON g_PlayerIcon;	// �v���C���[�̃A�C�R��
static ICON g_enemyIcon;	// �G�̃A�C�R��

static bool g_Load;

//=============================================================================
// ����������
//=============================================================================
HRESULT InitIcon(void)
{
	//�e�N�X�`������
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		g_Texture[i] = NULL;
		D3DX11CreateShaderResourceViewFromFile(GetDevice(),
			g_TexturName[i],
			NULL,
			NULL,
			&g_Texture[i],
			NULL);
	}


	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// �v���C���[�̃A�C�R��
	g_PlayerIcon.w = TEXTURE_WIDTH;
	g_PlayerIcon.h = TEXTURE_HEIGHT;
	g_PlayerIcon.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_PlayerIcon.texNo = 0;
	g_PlayerIcon.use = true;

	// �G�̃A�C�R��
	g_enemyIcon.w = TEXTURE_WIDTH;
	g_enemyIcon.h = TEXTURE_HEIGHT;
	g_enemyIcon.pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_enemyIcon.texNo = 1;
	g_enemyIcon.use = true;

	// ���_�o�b�t�@�ɒl���Z�b�g����
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	// ���_���W�̐ݒ�
	vertex[0].Position = XMFLOAT3(-g_PlayerIcon.w / 2.0f, 0.0f, g_PlayerIcon.h / 2.0f);
	vertex[1].Position = XMFLOAT3( g_PlayerIcon.w / 2.0f, 0.0f, g_PlayerIcon.h / 2.0f);
	vertex[2].Position = XMFLOAT3(-g_PlayerIcon.w / 2.0f, 0.0f,	-g_PlayerIcon.h / 2.0f);
	vertex[3].Position = XMFLOAT3(g_PlayerIcon.w / 2.0f, 0.0f,	-g_PlayerIcon.h / 2.0f);

	// �g�U���̐ݒ�
	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// �@���̐ݒ�
	vertex[0].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

	// �e�N�X�`�����W�̐ݒ�
	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);
	
	g_Load = TRUE;

	return S_OK;
}

//=============================================================================
// �I������
//=============================================================================
void UninitIcon(void)
{
	if (!g_Load) return;

	// ���_�o�b�t�@�̊J��
	if (g_VertexBuffer != NULL)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}
	
	// �e�N�X�`���̊J��
	for (int i = 0; i < TEXTURE_MAX; i++)
	{
		if (g_Texture[i] != NULL)
		{
			g_Texture[i]->Release();
			g_Texture[i] = NULL;
		}
	}

	g_Load = FALSE;
}

//=============================================================================
// �X�V����
//=============================================================================
void UpdateIcon(void)
{
	if (g_PlayerIcon.use)
	{
		PLAYER* player = GetPlayer();
		g_PlayerIcon.pos = XMFLOAT3(player->pos.x, player->pos.y + 200.0f, player->pos.z);
	}

	if (g_enemyIcon.use)
	{
		ENEMY* enemy = GetEnemy();
		g_enemyIcon.pos = XMFLOAT3(enemy->pos.x, enemy->pos.y + 400.0f, enemy->pos.z);
	}
}

//=============================================================================
// �`�揈��
//=============================================================================
void DrawIcon(void)
{
	if (!g_Load) return;

	if (g_PlayerIcon.use)
	{
		// ���[���h�s��̐ݒ�
		XMMATRIX mtxWorld, mtxScl, mtxTrans;
		mtxWorld = XMMatrixIdentity();
		mtxScl = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		mtxTrans = XMMatrixTranslation(g_PlayerIcon.pos.x, g_PlayerIcon.pos.y, g_PlayerIcon.pos.z);
		mtxWorld = XMMatrixMultiply(mtxScl, mtxWorld);
		mtxWorld = XMMatrixMultiply(mtxTrans, mtxWorld);

		// ���[���h�s���萔�o�b�t�@�ɃZ�b�g
		SetWorldMatrix(&mtxWorld);

		// ���_�o�b�t�@���Z�b�g
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// �e�N�X�`�����Z�b�g
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_PlayerIcon.texNo]);

		// �v���~�e�B�u�g�|���W�ݒ�
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// �|���S���̕`��
		GetDeviceContext()->Draw(4, 0);
	}

	if (g_enemyIcon.use)
	{
		// ���[���h�s��̐ݒ�
		XMMATRIX mtxWorld, mtxScl, mtxTrans;
		mtxWorld = XMMatrixIdentity();
		mtxScl = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		mtxTrans = XMMatrixTranslation(g_enemyIcon.pos.x, g_enemyIcon.pos.y, g_enemyIcon.pos.z);
		mtxWorld = XMMatrixMultiply(mtxScl, mtxWorld);
		mtxWorld = XMMatrixMultiply(mtxTrans, mtxWorld);

		// ���[���h�s���萔�o�b�t�@�ɃZ�b�g
		SetWorldMatrix(&mtxWorld);

		// ���_�o�b�t�@���Z�b�g
		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

		// �e�N�X�`�����Z�b�g
		GetDeviceContext()->PSSetShaderResources(0, 1, &g_Texture[g_enemyIcon.texNo]);

		// �v���~�e�B�u�g�|���W�ݒ�
		GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// �|���S���̕`��
		GetDeviceContext()->Draw(4, 0);
	}
}
