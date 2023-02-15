//=============================================================================
//
// SSAO���� [SSAO.h]
// Author : �с@���
//
//=============================================================================
#pragma once

//*****************************************************************************
// �C���N���[�h�t�@�C��
//*****************************************************************************
#include "main.h"
#include "renderer.h"
#include "camera.h"

//*****************************************************************************
// �}�N����`
//*****************************************************************************
enum SSAO_PASS
{
	NormalZMap,
	INSTNormalZMap,
	GrassNormalZMap,

	SSAO_PASS_MAX,
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitSSAO();
void UpdateSSAO();
void DrawSSAO();
void UninitSSAO();

void DrawNormalZMap();
void DrawSSAOTex();
void DrawSSAOBlurTex();

ID3D11PixelShader* GetSSAOPixelShader(int pass);
ID3D11VertexShader* GetSSAOVertexShader(int pass);
ID3D11InputLayout* GetSSAOInputLayout(int pass);
