//=============================================================================
//
// レンダリング処理 [renderer.h]
// Author : 林　劭羲
//
//=============================================================================
#pragma once


//*********************************************************
// マクロ定義
//*********************************************************
#define LIGHT_MAX		(5)
#define SHADOW_MAP_SIZE_X (1024 * 5)			// シャドウマップのサイズ
#define SHADOW_MAP_SIZE_Y (1024 * 5)			// シャドウマップのサイズ

enum LIGHT_TYPE
{
	LIGHT_TYPE_NONE,		//ライト無し
	LIGHT_TYPE_DIRECTIONAL,	//ディレクショナルライト
	LIGHT_TYPE_POINT,		//ポイントライト

	LIGHT_TYPE_NUM
};

enum BLEND_MODE
{
	BLEND_MODE_NONE,		//ブレンド無し
	BLEND_MODE_ALPHABLEND,	//αブレンド
	BLEND_MODE_ADD,			//加算ブレンド
	BLEND_MODE_SUBTRACT,	//減算ブレンド

	BLEDD_MODE_NUM
};

enum CULL_MODE
{
	CULL_MODE_NONE,			//カリング無し
	CULL_MODE_FRONT,		//表のポリゴンを描画しない(CW)
	CULL_MODE_BACK,			//裏のポリゴンを描画しない(CCW)

	CULL_MODE_NUM
};

enum SHADER_MODE
{
	SHADER_MODE_DEFAULT,			//デフォルトシェーダー
	SHADER_MODE_INSTANCING, 		//インスタンシングシェーダー
	SHADER_MODE_GRASS, 				//草シェーダー
	SHADER_MODE_SHADOW_MAP,			//シャドウマップ生成
	SHADER_MODE_SHADOW_MAP_INST,	//シャドウマップ生成(インスタンシング)
	SHADER_MODE_SHADOW_MAP_GRASS, 	//シャドウマップ生成(草)
	SHADER_MODE_SKY, 				//スカイシェーダー

	SHADER_MODE_NUM
};


//*********************************************************
// 構造体
//*********************************************************

// 頂点構造体
class VERTEX_3D
{
	public:
		XMFLOAT3	Position;
		XMFLOAT3	Normal;
		XMFLOAT4	Diffuse;
		XMFLOAT2	TexCoord;
};

// マテリアル構造体
class MATERIAL
{
	public:
		XMFLOAT4	Ambient;
		XMFLOAT4	Diffuse;
		XMFLOAT4	Specular;
		XMFLOAT4	Emission;
		float		Shininess;
		int			noTexSampling;
};

// ライト構造体
class LIGHT
{
	public:
		XMFLOAT3	Direction;	// ライトの方向
		XMFLOAT3	Position;	// ライトの位置
		XMFLOAT4	Diffuse;	// 拡散光の色
		XMFLOAT4	Ambient;	// 環境光の色
		float		Attenuation;// 減衰率
		int			Type;		// ライト種別・有効フラグ
		int			Enable;		// ライト種別・有効フラグ
};

// フォグ構造体
class FOG
{
	public:
		float		FogStart;	// フォグの開始距離
		float		FogEnd;		// フォグの最大距離
		XMFLOAT4	FogColor;	// フォグの色
};

class LIGHT_MATRIX
{
	public:
		XMMATRIX	LightView;
		XMMATRIX	LightProjection;
};

// インスタンス構造体
class INSTANCE
{
	public:
		XMFLOAT4	pos;
		XMFLOAT4	scl;
		XMFLOAT4	rot;
};



//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************
HRESULT InitRenderer(HINSTANCE hInstance, HWND hWnd, BOOL bWindow);
void UninitRenderer(void);

void Clear(void);
void Present(void);

ID3D11Device *GetDevice( void );
ID3D11DeviceContext *GetDeviceContext( void );

void SetDepthEnable( bool Enable );
void SetBlendState(BLEND_MODE bm);
void SetCullingMode(CULL_MODE cm);
void SetAlphaTestEnable(BOOL flag);

void SetWorldViewProjection2D( void );
void SetWorldMatrix( XMMATRIX *WorldMatrix );
void SetViewMatrix( XMMATRIX *ViewMatrix );
void SetProjectionMatrix( XMMATRIX *ProjectionMatrix );

void SetMaterial( MATERIAL material );

void SetLightEnable(BOOL flag);
void SetLight(int index, LIGHT* light);

void SetFogEnable(BOOL flag);
void SetFog(FOG* fog);

void DebugTextOut(char* text, int x, int y);

void SetFuchi(int flag);
void SetShaderCamera(XMFLOAT3 pos);

void SetShaderMode(int mode);

void SetLightMatrix(int index, LIGHT_MATRIX* lightMatrix);
void SetDefaultRenderTarget(void);
void SetShadowMapRenderTarget(void);
void SetShadowMapTexture(void);

void SetTime(float time);
void SetSSAO(bool flag);