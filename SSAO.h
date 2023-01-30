#pragma once
#include "main.h"
#include "renderer.h"
#include "camera.h"

enum SSAO_PASS
{
	NormalZMap,
	INSTNormalZMap,
	GrassNormalZMap,

	SSAO_PASS_MAX,
};

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
