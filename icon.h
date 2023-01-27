#pragma once
#include "main.h"
#include "renderer.h"
#include "sprite.h"

struct ICON
{
	bool use;
	float w, h;
	XMFLOAT3 pos;
	int texNo;
};

//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitIcon(void);
void UninitIcon(void);
void UpdateIcon(void);
void DrawIcon(void);
