#include "shadowmap.h"
#include "stage.h"
#include "tree.h"
#include "grass.h"

void InitShadowMap()
{
}

void UninitShadowMap()
{
}

void UpdateShadowMap()
{
}

void DrawShadowMap()
{
	// �V�F�[�_�[�ݒ�
	SetShaderMode(SHADER_MODE_SHADOW_MAP);
	SetShadowMapRenderTarget();

	DrawStage();
	DrawTreeShadowMap();
	DrawGrassShadowMap();
	
	
	// �����_�[�^�[�Q�b�g��߂�
	SetShaderMode(SHADER_MODE_DEFAULT);
	SetDefaultRenderTarget();
	SetShadowMapTexture();
}
