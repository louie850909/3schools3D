//=============================================================================
//
// �؏��� [grass.h]
// Author : 
//
//=============================================================================
#pragma once

#define	MAX_GRASS			(50000)			// ���ő吔
//*****************************************************************************
// �v���g�^�C�v�錾
//*****************************************************************************
HRESULT InitGrass(void);
void UninitGrass(void);
void UpdateGrass(void);
void DrawGrass(void);
void DrawGrassShadowMap(void);
void DrawGrassSSAO(int pass);

INSTANCE* getGrassInstance();


