#pragma once
#include "main.h"
#include "renderer.h"
#include "camera.h"

#define OCC_RADIUS 0.5f
#define OCC_FADE_START 0.2f
#define OCC_FADE_END 0.5f
#define SURFACE_EPSILON 0.05f

enum SSAO_PASS
{
	NormalZMap,
	INSTNormalZMap,
	ViewPosMap,
	INSTViewPosMap,

	SSAO_PASS_MAX,
};

struct SSAO_CONSTANT_BUFFER
{
	XMMATRIX ViewToTex;
	XMFLOAT4 OffsetVectors[14];
	XMFLOAT4 FrustumCorners[4];
	
	float OcclusionRadius = OCC_RADIUS;
	float OcclusionFadeStart = OCC_FADE_START;
	float OcclusionFadeEnd = OCC_FADE_END;
	float SurfaceEpsilon = SURFACE_EPSILON;
};

HRESULT InitSSAO();
void UpdateSSAO();
void DrawSSAO();
void UninitSSAO();

void DrawNormalZMap();
void DrawViewPosMap();
void DrawSSAOTex();
void DrawSSAOBlurTex();

ID3D11PixelShader* GetSSAOPixelShader(int pass);
ID3D11VertexShader* GetSSAOVertexShader(int pass);
ID3D11InputLayout* GetSSAOInputLayout(int pass);
