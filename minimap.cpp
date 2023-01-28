
#include "minimap.h"
#include "player.h"
#include "stage.h"
#include "sprite.h"
#include "tree.h"
#include "icon.h"

static ID3D11Texture2D*				g_MiniMap = NULL;
static ID3D11Texture2D*				g_MiniMapDS = NULL;
static D3D11_VIEWPORT				g_MiniMapViewport;
static ID3D11RenderTargetView*		g_MiniMapRTV = NULL;
static ID3D11DepthStencilView*		g_MiniMapDSV = NULL;
static ID3D11ShaderResourceView*	g_MiniMapSRV = NULL;

static ID3D11PixelShader*			g_PixelShaderMiniMap = NULL;

static ID3D11Buffer*				g_VertexBuffer = NULL;

void InitMiniMap()
{
	// ミニマップ用のテクスチャを作成
	D3D11_TEXTURE2D_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(texDesc));
	texDesc.Width = 256;
	texDesc.Height = 256;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;
	GetDevice()->CreateTexture2D(&texDesc, NULL, &g_MiniMap);

	// ミニマップ用のレンダーターゲットビューを作成
	GetDevice()->CreateRenderTargetView(g_MiniMap, NULL, &g_MiniMapRTV);

	// ミニマップ用のシェーダーリソースビューを作成
	GetDevice()->CreateShaderResourceView(g_MiniMap, NULL, &g_MiniMapSRV);

	// ミニマップ用の深度ステンシルテクスチャを作成
	texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GetDevice()->CreateTexture2D(&texDesc, NULL, &g_MiniMapDS);

	// ミニマップ用の深度ステンシルビューを作成
	GetDevice()->CreateDepthStencilView(g_MiniMapDS, NULL, &g_MiniMapDSV);

	// ミニマップ用のビューポートを作成
	g_MiniMapViewport.TopLeftX = 0;
	g_MiniMapViewport.TopLeftY = 0;
	g_MiniMapViewport.Width = 256;
	g_MiniMapViewport.Height = 256;
	g_MiniMapViewport.MinDepth = 0.0f;
	g_MiniMapViewport.MaxDepth = 1.0f;

	// 頂点バッファ生成
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);
	
	// ミニマップ用のピクセルシェーダーを作成
	ID3DBlob* pPSBlob = NULL;
	ID3DBlob* pErrorBlob;
	DWORD shFlag = D3DCOMPILE_ENABLE_STRICTNESS;
	
#if defined(_DEBUG)
	shFlag |= D3DCOMPILE_DEBUG;
#endif
	
	HRESULT hr = D3DX11CompileFromFile("minimap.hlsl", NULL, NULL, "MiniMapPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}

	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderMiniMap);

	pPSBlob->Release();
	
	InitIcon();
}

void UninitMiniMap()
{
	if (g_PixelShaderMiniMap)
	{
		g_PixelShaderMiniMap->Release();
		g_PixelShaderMiniMap = NULL;
	}

	if (g_MiniMapDSV)
	{
		g_MiniMapDSV->Release();
		g_MiniMapDSV = NULL;
	}

	if (g_MiniMapDS)
	{
		g_MiniMapDS->Release();
		g_MiniMapDS = NULL;
	}

	if (g_MiniMapSRV)
	{
		g_MiniMapSRV->Release();
		g_MiniMapSRV = NULL;
	}

	if (g_MiniMapRTV)
	{
		g_MiniMapRTV->Release();
		g_MiniMapRTV = NULL;
	}

	if (g_MiniMap)
	{
		g_MiniMap->Release();
		g_MiniMap = NULL;
	}

	UninitIcon();
}

void UpdateMiniMap()
{
	UpdateIcon();
}

void DrawMiniMap()
{
	// ピクセルシェーダーを設定
	GetDeviceContext()->PSSetShader(g_PixelShaderMiniMap, NULL, 0);

	// 頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);

	// マトリクス設定
	SetWorldViewProjection2D();

	// プリミティブトポロジ設定
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	
	SetSpriteLeftTop(g_VertexBuffer, 684.0f, 20.0f, 256.0f, 256.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	GetDeviceContext()->Draw(4, 0);

	// シェーダーを元に戻す
	SetShaderMode(SHADER_MODE_DEFAULT);
}

void DrawMiniMapTex()
{
	CAMERA* camera = GetCamera();
	XMFLOAT3 oldPos = camera->pos;
	XMFLOAT3 oldRot = camera->rot;
	XMFLOAT3 oldAt = camera->at;

	STAGE* stage = GetStage();
	XMFLOAT3 stageRot = stage->rot;
	
	// カメラを上空から見下ろすように設定
	// プレイヤーの位置を中心にする
	PLAYER* player = GetPlayer();
	SetCameraPosAtRot(XMFLOAT3(player->pos.x + 0.001f, player->pos.y + 1000.0f, player->pos.z + 0.001f), player->pos, XMFLOAT3(0.0f, 0.0f, 0.0f));
	SetCamera();
	
	XMMATRIX orthoProj = XMMatrixOrthographicLH(2000.0f, 2000.0f, 0.1f, 5000.0f);
	SetProjectionMatrix(&orthoProj);
	
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(2, 1, null);

	// ミニマップ用のレンダーターゲットビューを設定
	GetDeviceContext()->RSSetViewports(1, &g_MiniMapViewport);
	GetDeviceContext()->OMSetRenderTargets(1, &g_MiniMapRTV, g_MiniMapDSV);
	GetDeviceContext()->ClearRenderTargetView(g_MiniMapRTV, clearColor);
	GetDeviceContext()->ClearDepthStencilView(g_MiniMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
	DrawStage();
	DrawTree();
	SetLightEnable(false);
	DrawIcon();
	SetLightEnable(true);

	// レンダーターゲットを戻す
	SetShaderMode(SHADER_MODE_DEFAULT);
	SetDefaultRenderTarget();

	SetCameraPosAtRot(oldPos, oldAt, oldRot);
	SetCamera();
	
	// シェーダーにテクスチャを設定する
	GetDeviceContext()->PSSetShaderResources(2, 1, &g_MiniMapSRV);
}
