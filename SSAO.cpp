//=============================================================================
//
// SSAO処理 [SSAO.cpp]
// Author : 林　劭羲
//
//=============================================================================
#include "SSAO.h"
#include "stage.h"
#include "tree.h"
#include "grass.h"
#include "camera.h"

//*****************************************************************************
// マクロ定義
//*****************************************************************************


//*****************************************************************************
// プロトタイプ宣言
//*****************************************************************************


//*****************************************************************************
// グローバル変数
//*****************************************************************************
static ID3D11Texture2D* g_NormalZMap = NULL;
static ID3D11Texture2D* g_NormalZMapDS = NULL;
static ID3D11RenderTargetView* g_NormalZMapRTV = NULL;
static ID3D11DepthStencilView* g_NormalZMapDSV = NULL;
static ID3D11ShaderResourceView* g_NormalZMapSRV = NULL;

static ID3D11Texture2D* g_SSAORandomTex = NULL;
static ID3D11ShaderResourceView* g_SSAORandomTexSRV = NULL;

static ID3D11Texture2D* g_SSAOTex = NULL;
static ID3D11RenderTargetView* g_SSAOTexRTV = NULL;
static ID3D11ShaderResourceView* g_SSAOTexSRV = NULL;

static ID3D11Texture2D* g_SSAOBlurTex = NULL;
static ID3D11RenderTargetView* g_SSAOBlurTexRTV = NULL;
static ID3D11ShaderResourceView* g_SSAOBlurTexSRV = NULL;

static ID3D11VertexShader*	g_SSAOInstVS = NULL;
static ID3D11InputLayout*	g_SSAOInstLayout = NULL;
static ID3D11PixelShader*	g_SSAOInstPS = NULL;

static ID3D11VertexShader*	g_SSAOGrassVS = NULL;
static ID3D11InputLayout*	g_SSAOGrassLayout = NULL;
static ID3D11PixelShader*	g_SSAOGrassPS = NULL;

static ID3D11VertexShader* g_VertexShaderSSAONormalZMap = NULL;
static ID3D11InputLayout* g_InputLayoutSSAONormalZMap = NULL;
static ID3D11PixelShader* g_PixelShaderSSAONormalZMap = NULL;

static ID3D11PixelShader* g_PixelShaderSSAO = NULL;

static ID3D11PixelShader* g_PixelShaderSSAOBlur = NULL;

static ID3D11Buffer* g_VertexBuffer = NULL;

//=============================================================================
// 初期化処理
//=============================================================================
HRESULT InitSSAO()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_IMMUTABLE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.SysMemPitch = 256 * 4;

	// ランダムテクスチャの作成
	{
		BYTE* pRandomTex = new BYTE[256 * 256 * 4];
		for (int i = 0; i < 256 * 256; i++)
		{
			XMFLOAT3 v;
			v.x = (float)(rand() % 256) / 255.0f;
			v.y = (float)(rand() % 256) / 255.0f;
			v.z = 0.0f;

			XMVECTOR vN = XMVector3Normalize(XMLoadFloat3(&v));

			XMStoreFloat3(&v, vN);

			pRandomTex[i * 4 + 0] = (BYTE)(v.x * 255.0f);
			pRandomTex[i * 4 + 1] = (BYTE)(v.y * 255.0f);
			pRandomTex[i * 4 + 2] = (BYTE)(v.z * 255.0f);
			pRandomTex[i * 4 + 3] = 0;
		}

		initData.pSysMem = pRandomTex;

		GetDevice()->CreateTexture2D(&texDesc, &initData, &g_SSAORandomTex);
		GetDevice()->CreateShaderResourceView(g_SSAORandomTex, NULL, &g_SSAORandomTexSRV);
		GetDeviceContext()->PSSetShaderResources(4, 1, &g_SSAORandomTexSRV);
		delete[] pRandomTex;
	}

	// 法線Zマップの作成
	{
		texDesc.Width = SCREEN_WIDTH;
		texDesc.Height = SCREEN_HEIGHT;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_NormalZMap);
		GetDevice()->CreateRenderTargetView(g_NormalZMap, NULL, &g_NormalZMapRTV);
		GetDevice()->CreateShaderResourceView(g_NormalZMap, NULL, &g_NormalZMapSRV);

		texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_NormalZMapDS);
		GetDevice()->CreateDepthStencilView(g_NormalZMapDS, NULL, &g_NormalZMapDSV);
	}

	// SSAOマップの作成
	{
		texDesc.Width = SCREEN_WIDTH;
		texDesc.Height = SCREEN_HEIGHT;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_SSAOTex);
		GetDevice()->CreateRenderTargetView(g_SSAOTex, NULL, &g_SSAOTexRTV);
		GetDevice()->CreateShaderResourceView(g_SSAOTex, NULL, &g_SSAOTexSRV);
	}

	// SSAOブラー用マップの作成
	{
		texDesc.Width = SCREEN_WIDTH;
		texDesc.Height = SCREEN_HEIGHT;
		texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_SSAOBlurTex);
		GetDevice()->CreateRenderTargetView(g_SSAOBlurTex, NULL, &g_SSAOBlurTexRTV);
		GetDevice()->CreateShaderResourceView(g_SSAOBlurTex, NULL, &g_SSAOBlurTexSRV);
	}

	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pErrorBlob;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	shFlag |= D3DCOMPILE_DEBUG;
#endif

	// インスタンシング頂点シェーダー
	HRESULT hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_INSTVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_SSAOInstVS);

	// 入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTSCL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTROT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};
	UINT numElements = ARRAYSIZE(layout);

	GetDevice()->CreateInputLayout(layout,
		numElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_SSAOInstLayout);

	pVSBlob->Release();
	pVSBlob = NULL;

	// インスタンシングピクセルシェーダー
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_INSTPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_SSAOInstPS);
	pPSBlob->Release();
	pPSBlob = NULL;

	// 草の頂点シェーダー
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_GRASSVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_SSAOGrassVS);
	
	// 草の入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC grassLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTSCL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTROT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};
	UINT grassNumElements = ARRAYSIZE(grassLayout);
	
	GetDevice()->CreateInputLayout(grassLayout,
		grassNumElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_SSAOGrassLayout);

	pVSBlob->Release();
	pVSBlob = NULL;

	// 草のピクセルシェーダー
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_GRASSPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_SSAOGrassPS);
	pPSBlob->Release();
	pPSBlob = NULL;

	// ノーマルZマップの頂点シェーダー
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "NormalZMapVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderSSAONormalZMap);

	// ノーマルZマップの入力レイアウト生成
	D3D11_INPUT_ELEMENT_DESC normalZMapLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT normalZMapNumElements = ARRAYSIZE(normalZMapLayout);
	
	GetDevice()->CreateInputLayout(normalZMapLayout,
		normalZMapNumElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_InputLayoutSSAONormalZMap);
	
	pVSBlob->Release();
	pVSBlob = NULL;
	
	// ノーマルZマップ用のピクセルシェーダーを作成
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "NormalZMapPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAONormalZMap);
	pPSBlob->Release();
	pPSBlob = NULL;
	
	// SSAO用のピクセルシェーダーを作成
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAOPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAO);
	pPSBlob->Release();
	pPSBlob = NULL;
	
	// SSAOブラー用のピクセルシェーダーを作成
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAOBlurPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAOBlur);
	pPSBlob->Release();
	pPSBlob = NULL;
	
	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);
	
	return S_OK;
}

//=============================================================================
// 更新処理
//=============================================================================
void UpdateSSAO()
{
}

//=============================================================================
// 描画処理
//=============================================================================
void DrawSSAO()
{
	DrawNormalZMap();
	DrawSSAOTex();
	DrawSSAOBlurTex();
}

//=============================================================================
// 終了処理
//=============================================================================
void UninitSSAO()
{
	if (g_NormalZMap)
	{
		g_NormalZMap->Release();
		g_NormalZMap = NULL;
	}

	if(g_NormalZMapDS)
	{
		g_NormalZMapDS->Release();
		g_NormalZMapDS = NULL;
	}
	
	if (g_NormalZMapRTV)
	{
		g_NormalZMapRTV->Release();
		g_NormalZMapRTV = NULL;
	}

	if (g_NormalZMapSRV)
	{
		g_NormalZMapSRV->Release();
		g_NormalZMapSRV = NULL;
	}
	
	if (g_NormalZMapDSV)
	{
		g_NormalZMapDSV->Release();
		g_NormalZMapDSV = NULL;
	}

	if (g_SSAORandomTex)
	{
		g_SSAORandomTex->Release();
		g_SSAORandomTex = NULL;
	}
	
	if (g_SSAORandomTexSRV)
	{
		g_SSAORandomTexSRV->Release();
		g_SSAORandomTexSRV = NULL;
	}

	if (g_SSAOTex)
	{
		g_SSAOTex->Release();
		g_SSAOTex = NULL;
	}
	
	if (g_SSAOTexRTV)
	{
		g_SSAOTexRTV->Release();
		g_SSAOTexRTV = NULL;
	}

	if (g_SSAOTexSRV)
	{
		g_SSAOTexSRV->Release();
		g_SSAOTexSRV = NULL;
	}

	if (g_SSAOBlurTex)
	{
		g_SSAOBlurTex->Release();
		g_SSAOBlurTex = NULL;
	}

	if (g_SSAOBlurTexRTV)
	{
		g_SSAOBlurTexRTV->Release();
		g_SSAOBlurTexRTV = NULL;
	}

	if (g_SSAOBlurTexSRV)
	{
		g_SSAOBlurTexSRV->Release();
		g_SSAOBlurTexSRV = NULL;
	}

	if (g_PixelShaderSSAONormalZMap)
	{
		g_PixelShaderSSAONormalZMap->Release();
		g_PixelShaderSSAONormalZMap = NULL;
	}

	if (g_PixelShaderSSAO)
	{
		g_PixelShaderSSAO->Release();
		g_PixelShaderSSAO = NULL;
	}

	if (g_PixelShaderSSAOBlur)
	{
		g_PixelShaderSSAOBlur->Release();
		g_PixelShaderSSAOBlur = NULL;
	}

	if (g_VertexBuffer)
	{
		g_VertexBuffer->Release();
		g_VertexBuffer = NULL;
	}

	if(g_SSAOInstVS)
	{
		g_SSAOInstVS->Release();
		g_SSAOInstVS = NULL;
	}
	
	if(g_SSAOInstPS)
	{
		g_SSAOInstPS->Release();
		g_SSAOInstPS = NULL;
	}
	
	if (g_SSAOInstLayout)
	{
		g_SSAOInstLayout->Release();
		g_SSAOInstLayout = NULL;
	}

	if(g_SSAOGrassVS)
	{
		g_SSAOGrassVS->Release();
		g_SSAOGrassVS = NULL;
	}
	
	if (g_SSAOGrassPS)
	{
		g_SSAOGrassPS->Release();
		g_SSAOGrassPS = NULL;
	}

	if (g_SSAOGrassLayout)
	{
		g_SSAOGrassLayout->Release();
		g_SSAOGrassLayout = NULL;
	}
}

//=============================================================================
// ノーマルZマップ描画
//=============================================================================
void DrawNormalZMap()
{
	// ピクセルシェーダーを設定
	GetDeviceContext()->IASetInputLayout(g_InputLayoutSSAONormalZMap);
	GetDeviceContext()->VSSetShader(g_VertexShaderSSAONormalZMap, NULL, 0);
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAONormalZMap, NULL, 0);
	
	// クリアシェーダーリソース
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(3, 1, null);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = SCREEN_WIDTH;
	vp.Height = SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// レンダーターゲットを設定
	GetDeviceContext()->OMSetRenderTargets(1, &g_NormalZMapRTV, g_NormalZMapDSV);
	
	// レンダリングターゲットをクリア
	float Clear[4] = { 0.0f, 0.0f, -1.0f, VIEW_FAR_Z };
	GetDeviceContext()->ClearRenderTargetView(g_NormalZMapRTV, Clear);
	GetDeviceContext()->ClearDepthStencilView(g_NormalZMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	DrawStage();

	DrawTreeSSAO(INSTNormalZMap);

	//DrawGrassSSAO(GrassNormalZMap);

	// レンダーターゲットを設定
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// シェーダーにテクスチャを設定する
	GetDeviceContext()->PSSetShaderResources(3, 1, &g_NormalZMapSRV);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

//=============================================================================
// SSAOテクスチャ描画
//=============================================================================
void DrawSSAOTex()
{
	// ピクセルシェーダーを設定
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAO, NULL, 0);

	// クリアシェーダーリソース
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(5, 1, null);

	// ビューポートを設定
	D3D11_VIEWPORT vp;
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// レンダーターゲットを設定
	GetDeviceContext()->OMSetRenderTargets(1, &g_SSAOTexRTV, NULL);

	// レンダリングターゲットをクリア
	float Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(g_SSAOTexRTV, Clear);

	// 頂点バッファを設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	SetWorldViewProjection2D();
	
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
	
	vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Position = XMFLOAT3(SCREEN_WIDTH, 0.0f, 0.0f);
	vertex[2].Position = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	vertex[3].Position = XMFLOAT3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	GetDeviceContext()->Draw(4, 0);

	// レンダーターゲットを設定
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// シェーダーにテクスチャを設定する
	GetDeviceContext()->PSSetShaderResources(5, 1, &g_SSAOTexSRV);
}

//=============================================================================
// SSAOブラー描画
//=============================================================================
void DrawSSAOBlurTex()
{
	// ピクセルシェーダーを設定
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAOBlur, NULL, 0);

	// クリアシェーダーリソース
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(6, 1, null);

	// ビューポートを設定
	D3D11_VIEWPORT vp;
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// レンダーターゲットを設定
	GetDeviceContext()->OMSetRenderTargets(1, &g_SSAOBlurTexRTV, NULL);

	// レンダリングターゲットをクリア
	float Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(g_SSAOBlurTexRTV, Clear);

	// 頂点バッファを設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	SetWorldViewProjection2D();

	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;

	vertex[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Position = XMFLOAT3(SCREEN_WIDTH, 0.0f, 0.0f);
	vertex[2].Position = XMFLOAT3(0.0f, SCREEN_HEIGHT, 0.0f);
	vertex[3].Position = XMFLOAT3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);

	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[1].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[2].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
	vertex[3].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	vertex[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	GetDeviceContext()->Unmap(g_VertexBuffer, 0);

	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	GetDeviceContext()->Draw(4, 0);

	// レンダーターゲットを設定
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// シェーダーにテクスチャを設定する
	GetDeviceContext()->PSSetShaderResources(6, 1, &g_SSAOBlurTexSRV);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

//=============================================================================
// SSAOピクセルシェーダー取得
//=============================================================================
ID3D11PixelShader* GetSSAOPixelShader(int pass)
{
	switch (pass)
	{
	case NormalZMap:
		return g_PixelShaderSSAONormalZMap;
		break;

	case INSTNormalZMap:
		return g_SSAOInstPS;
		break;

	case GrassNormalZMap:
		return g_SSAOGrassPS;
		break;
	}
}

//=============================================================================
// SSAO頂点シェーダー取得
//=============================================================================
ID3D11VertexShader* GetSSAOVertexShader(int pass)
{
	switch (pass)
	{
	case INSTNormalZMap:
		return g_SSAOInstVS;
		break;

	case GrassNormalZMap:
		return g_SSAOGrassVS;
		break;
	}
}

ID3D11InputLayout* GetSSAOInputLayout(int pass)
{
	switch (pass)
	{
	case INSTNormalZMap:
		return g_SSAOInstLayout;
		break;

	case GrassNormalZMap:
		return g_SSAOGrassLayout;
		break;
	}
}
