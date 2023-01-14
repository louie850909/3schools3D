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
	// シェーダー設定
	SetShaderMode(SHADER_MODE_SHADOW_MAP);
	SetShadowMapRenderTarget();

	DrawStage();
	DrawTreeShadowMap();
	DrawGrassShadowMap();
	
	
	// レンダーターゲットを戻す
	SetShaderMode(SHADER_MODE_DEFAULT);
	SetDefaultRenderTarget();
	SetShadowMapTexture();
}
