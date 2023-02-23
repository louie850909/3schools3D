#pragma once
#include "main.h"
#include "renderer.h"
#include "camera.h"

#define OCC_RADIUS 0.8f
#define OCC_FADE_START 0.1f
#define OCC_FADE_END 0.5f
#define SURFACE_EPSILON 0.01f

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
	XMFLOAT4 FrustumCorners[4];
	
	float OcclusionRadius;
	float OcclusionFadeStart;
	float OcclusionFadeEnd;
	float SurfaceEpsilon;
};

struct SSAO_OFFSET_VECTORS
{
	XMFLOAT4 OffsetVectors[14];
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
