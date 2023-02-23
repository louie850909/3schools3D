#include "SSAO.h"
#include "stage.h"
#include "tree.h"
#include "grass.h"
#include "camera.h"
#include <time.h>

// SSAO���\�[�X
static ID3D11Texture2D* g_NormalZMap = NULL;
static ID3D11Texture2D* g_NormalZMapDS = NULL;
static ID3D11RenderTargetView* g_NormalZMapRTV = NULL;
static ID3D11DepthStencilView* g_NormalZMapDSV = NULL;
static ID3D11ShaderResourceView* g_NormalZMapSRV = NULL;

static ID3D11Texture2D* g_ViewPosMap = NULL;
static ID3D11Texture2D* g_ViewPosMapDS = NULL;
static ID3D11RenderTargetView* g_ViewPosMapRTV = NULL;
static ID3D11DepthStencilView* g_ViewPosMapDSV = NULL;
static ID3D11ShaderResourceView* g_ViewPosMapSRV = NULL;

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

static ID3D11VertexShader* g_VertexShaderSSAONormalZMap = NULL;
static ID3D11InputLayout* g_InputLayoutSSAONormalZMap = NULL;
static ID3D11PixelShader* g_PixelShaderSSAONormalZMap = NULL;

static ID3D11VertexShader* g_VertexShaderSSAO = NULL;
static ID3D11InputLayout* g_InputLayoutSSAO = NULL;
static ID3D11PixelShader* g_PixelShaderSSAO = NULL;

static ID3D11PixelShader* g_PixelShaderSSAOBlur = NULL;

static ID3D11VertexShader* g_VertexShaderViewPosMap = NULL;
static ID3D11InputLayout* g_InputLayoutViewPosMap = NULL;
static ID3D11PixelShader* g_PixelShaderViewPosMap = NULL;

static ID3D11VertexShader* g_VertexShaderInstViewPosMap = NULL;
static ID3D11InputLayout* g_InputLayoutInstViewPosMap = NULL;
static ID3D11PixelShader* g_PixelShaderInstViewPosMap = NULL;

static ID3D11Buffer* g_VertexBuffer = NULL;

static ID3D11Buffer* g_SSAOConstantBuffer = NULL;
static ID3D11Buffer* g_SSAOOffsetBuffer = NULL;

static SSAO_CONSTANT_BUFFER g_SSAOConstant;
static SSAO_OFFSET_VECTORS g_SSAOOffset;

void SetSSAOConstant(void);
void SetSSAOOffsetVectors(void);
static XMMATRIX InverseTranspose(CXMMATRIX M);

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

	// �����_���e�N�X�`���̍쐬
	{
		BYTE* pRandomTex = new BYTE[256 * 256 * 4];
		for (int i = 0; i < 256 * 256; i++)
		{
			XMFLOAT3 v;
			v.x = (float)(rand() % 256) / 255.0f;
			v.y = (float)(rand() % 256) / 255.0f;
			v.z = (float)(rand() % 256) / 255.0f;

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

	// �@��Z�}�b�v�̍쐬
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

	// View��ԍ��W�}�b�v�̍쐬
	{
		texDesc.Width = SCREEN_WIDTH;
		texDesc.Height = SCREEN_HEIGHT;
		texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_ViewPosMap);
		GetDevice()->CreateRenderTargetView(g_ViewPosMap, NULL, &g_ViewPosMapRTV);
		GetDevice()->CreateShaderResourceView(g_ViewPosMap, NULL, &g_ViewPosMapSRV);

		texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		GetDevice()->CreateTexture2D(&texDesc, NULL, &g_ViewPosMapDS);
		GetDevice()->CreateDepthStencilView(g_ViewPosMapDS, NULL, &g_ViewPosMapDSV);
	}

	// SSAO�}�b�v�̍쐬
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

	// SSAO�u���[�p�}�b�v�̍쐬
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
	shFlag |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// �C���X�^���V���O���_�V�F�[�_�[
	HRESULT hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_INSTVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}

	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_SSAOInstVS);

	// ���̓��C�A�E�g����
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

	// �C���X�^���V���O�s�N�Z���V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAO_INSTPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_SSAOInstPS);
	pPSBlob->Release();
	pPSBlob = NULL;

	// View��ԍ��W���_�V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "ViewPosMapVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderViewPosMap);

	// View��ԍ��W�̓��̓��C�A�E�g����
	D3D11_INPUT_ELEMENT_DESC ViewPosMapLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	GetDevice()->CreateInputLayout(ViewPosMapLayout,
				ARRAYSIZE(ViewPosMapLayout),
				pVSBlob->GetBufferPointer(),
				pVSBlob->GetBufferSize(),
				&g_InputLayoutViewPosMap);

	pVSBlob->Release();
	pVSBlob = NULL;

	// View��ԍ��W�s�N�Z���V�F�[�_
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "ViewPosMapPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderViewPosMap);
	pPSBlob->Release();
	pPSBlob = NULL;

	// View��ԃC���X�^���V���O���W���_�V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "InstViewPosMapVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderInstViewPosMap);
	
	// View��ԃC���X�^���V���O���W�̓��̓��C�A�E�g����
	D3D11_INPUT_ELEMENT_DESC InstViewPosMapLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTSCL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		{ "INSTROT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,    1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
	};
	UINT grassNumElements = ARRAYSIZE(InstViewPosMapLayout);
	
	GetDevice()->CreateInputLayout(InstViewPosMapLayout,
		grassNumElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_InputLayoutInstViewPosMap);

	pVSBlob->Release();
	pVSBlob = NULL;

	// View��ԃC���X�^���V���O���W�̃s�N�Z���V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "InstViewPosMapPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderInstViewPosMap);
	pPSBlob->Release();
	pPSBlob = NULL;

	// �m�[�}��Z�}�b�v�̒��_�V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "NormalZMapVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderSSAONormalZMap);

	// �m�[�}��Z�}�b�v�̓��̓��C�A�E�g����
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
	
	// �m�[�}��Z�}�b�v�p�̃s�N�Z���V�F�[�_�[���쐬
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "NormalZMapPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAONormalZMap);
	pPSBlob->Release();
	pPSBlob = NULL;

	// SSAO�p�̒��_�V�F�[�_�[
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAOVS", "vs_4_0", shFlag, 0, NULL, &pVSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "VS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &g_VertexShaderSSAO);
	
	// SSAO�p�̓��̓��C�A�E�g����
	D3D11_INPUT_ELEMENT_DESC SSAOLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,          0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT SSAONumElements = ARRAYSIZE(SSAOLayout);
	
	GetDevice()->CreateInputLayout(SSAOLayout,
		SSAONumElements,
		pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(),
		&g_InputLayoutSSAO);
	
	pVSBlob->Release();
	pVSBlob = NULL;
	
	// SSAO�p�̃s�N�Z���V�F�[�_�[���쐬
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAOPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAO);
	pPSBlob->Release();
	pPSBlob = NULL;
	
	// SSAO�u���[�p�̃s�N�Z���V�F�[�_�[���쐬
	hr = D3DX11CompileFromFile("SSAO.hlsl", NULL, NULL, "SSAOBlurPS", "ps_4_0", shFlag, 0, NULL, &pPSBlob, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		MessageBox(NULL, (char*)pErrorBlob->GetBufferPointer(), "PS", MB_OK | MB_ICONERROR);
	}
	GetDevice()->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &g_PixelShaderSSAOBlur);
	pPSBlob->Release();
	pPSBlob = NULL;
	
	// ���_�o�b�t�@����
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX_3D) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	GetDevice()->CreateBuffer(&bd, NULL, &g_VertexBuffer);

	// �R���X�^���g�o�b�t�@����
	D3D11_BUFFER_DESC hBufferDesc;
	hBufferDesc.ByteWidth = sizeof(SSAO_CONSTANT_BUFFER);
	hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hBufferDesc.CPUAccessFlags = 0;
	hBufferDesc.MiscFlags = 0;
	hBufferDesc.StructureByteStride = sizeof(float);

	GetDevice()->CreateBuffer(&hBufferDesc, NULL, &g_SSAOConstantBuffer);
	GetDeviceContext()->VSSetConstantBuffers(11, 1, &g_SSAOConstantBuffer);
	GetDeviceContext()->PSSetConstantBuffers(11, 1, &g_SSAOConstantBuffer);

	hBufferDesc.ByteWidth = sizeof(SSAO_OFFSET_VECTORS);

	GetDevice()->CreateBuffer(&hBufferDesc, NULL, &g_SSAOOffsetBuffer);
	GetDeviceContext()->VSSetConstantBuffers(12, 1, &g_SSAOOffsetBuffer);
	GetDeviceContext()->PSSetConstantBuffers(12, 1, &g_SSAOOffsetBuffer);

	SetSSAOConstant();
	SetSSAOOffsetVectors();
	
	return S_OK;
}

void UpdateSSAO()
{
}

void DrawSSAO()
{
	DrawNormalZMap();
	DrawViewPosMap();
	DrawSSAOTex();
	DrawSSAOBlurTex();
}

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

	if (g_VertexShaderSSAONormalZMap)
	{
		g_VertexShaderSSAONormalZMap->Release();
		g_VertexShaderSSAONormalZMap = NULL;
	}

	if (g_InputLayoutSSAONormalZMap)
	{
		g_InputLayoutSSAONormalZMap->Release();
		g_InputLayoutSSAONormalZMap = NULL;
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

	if (g_SSAOConstantBuffer)
	{
		g_SSAOConstantBuffer->Release();
		g_SSAOConstantBuffer = NULL;
	}

	if (g_VertexShaderViewPosMap)
	{
		g_VertexShaderViewPosMap->Release();
		g_VertexShaderViewPosMap = NULL;
	}

	if (g_InputLayoutViewPosMap)
	{
		g_InputLayoutViewPosMap->Release();
		g_InputLayoutViewPosMap = NULL;
	}

	if (g_PixelShaderViewPosMap)
	{
		g_PixelShaderViewPosMap->Release();
		g_PixelShaderViewPosMap = NULL;
	}

	if (g_VertexShaderInstViewPosMap)
	{
		g_VertexShaderInstViewPosMap->Release();
		g_VertexShaderInstViewPosMap = NULL;
	}

	if (g_InputLayoutInstViewPosMap)
	{
		g_InputLayoutInstViewPosMap->Release();
		g_InputLayoutInstViewPosMap = NULL;
	}

	if (g_PixelShaderInstViewPosMap)
	{
		g_PixelShaderInstViewPosMap->Release();
		g_PixelShaderInstViewPosMap = NULL;
	}
}

void DrawNormalZMap()
{
	// �s�N�Z���V�F�[�_�[��ݒ�
	GetDeviceContext()->IASetInputLayout(g_InputLayoutSSAONormalZMap);
	GetDeviceContext()->VSSetShader(g_VertexShaderSSAONormalZMap, NULL, 0);
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAONormalZMap, NULL, 0);
	
	// �N���A�V�F�[�_�[���\�[�X
	ID3D11ShaderResourceView* null[] = { nullptr };
	GetDeviceContext()->PSSetShaderResources(3, 1, null);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = SCREEN_WIDTH;
	vp.Height = SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// �����_�[�^�[�Q�b�g��ݒ�
	GetDeviceContext()->OMSetRenderTargets(1, &g_NormalZMapRTV, g_NormalZMapDSV);
	
	// �����_�����O�^�[�Q�b�g���N���A
	float Clear[4] = { 0.0f, 0.0f, -1.0f, VIEW_FAR_Z };
	GetDeviceContext()->ClearRenderTargetView(g_NormalZMapRTV, Clear);
	GetDeviceContext()->ClearDepthStencilView(g_NormalZMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	DrawStage();

	DrawTreeSSAO(INSTNormalZMap);

	// �����_�[�^�[�Q�b�g��ݒ�
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// �V�F�[�_�[�Ƀe�N�X�`����ݒ肷��
	GetDeviceContext()->PSSetShaderResources(3, 1, &g_NormalZMapSRV);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

void DrawViewPosMap()
{
	// �s�N�Z���V�F�[�_�[��ݒ�
	GetDeviceContext()->IASetInputLayout(g_InputLayoutViewPosMap);
	GetDeviceContext()->VSSetShader(g_VertexShaderViewPosMap, NULL, 0);
	GetDeviceContext()->PSSetShader(g_PixelShaderViewPosMap, NULL, 0);

	// �N���A�V�F�[�_�[���\�[�X
	ID3D11ShaderResourceView* null[] = { nullptr };
	GetDeviceContext()->PSSetShaderResources(7, 1, null);

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = SCREEN_WIDTH;
	vp.Height = SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// �����_�[�^�[�Q�b�g��ݒ�
	GetDeviceContext()->OMSetRenderTargets(1, &g_ViewPosMapRTV, g_ViewPosMapDSV);

	// �����_�����O�^�[�Q�b�g���N���A
	float Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(g_ViewPosMapRTV, Clear);
	GetDeviceContext()->ClearDepthStencilView(g_ViewPosMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	DrawStage();
	DrawTreeSSAO(INSTViewPosMap);

	// �����_�[�^�[�Q�b�g��ݒ�
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// �V�F�[�_�[�Ƀe�N�X�`����ݒ肷��
	GetDeviceContext()->PSSetShaderResources(7, 1, &g_ViewPosMapSRV);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

void DrawSSAOTex()
{
	// �s�N�Z���V�F�[�_�[��ݒ�
	GetDeviceContext()->IASetInputLayout(g_InputLayoutSSAO);
	GetDeviceContext()->VSSetShader(g_VertexShaderSSAO, NULL, 0);
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAO, NULL, 0);

	// �N���A�V�F�[�_�[���\�[�X
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(5, 1, null);

	// �r���[�|�[�g��ݒ�
	D3D11_VIEWPORT vp;
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// �����_�[�^�[�Q�b�g��ݒ�
	GetDeviceContext()->OMSetRenderTargets(1, &g_SSAOTexRTV, NULL);

	// �����_�����O�^�[�Q�b�g���N���A
	float Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(g_SSAOTexRTV, Clear);

	// ���_�o�b�t�@��ݒ�
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	GetDeviceContext()->IASetVertexBuffers(0, 1, &g_VertexBuffer, &stride, &offset);
	
	D3D11_MAPPED_SUBRESOURCE msr;
	GetDeviceContext()->Map(g_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

	VERTEX_3D* vertex = (VERTEX_3D*)msr.pData;
	
	vertex[0].Position = XMFLOAT3(-1.0f, +1.0f, 0.0f);
	vertex[1].Position = XMFLOAT3(+1.0f, +1.0f, 0.0f);
	vertex[2].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertex[3].Position = XMFLOAT3(+1.0f, -1.0f, 0.0f);

	vertex[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertex[1].Normal = XMFLOAT3(1.0f, 0.0f, 0.0f);
	vertex[2].Normal = XMFLOAT3(2.0f, 0.0f, 0.0f);
	vertex[3].Normal = XMFLOAT3(3.0f, 0.0f, 0.0f);

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

	// �����_�[�^�[�Q�b�g��ݒ�
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// �V�F�[�_�[�Ƀe�N�X�`����ݒ肷��
	GetDeviceContext()->PSSetShaderResources(5, 1, &g_SSAOTexSRV);
}

void DrawSSAOBlurTex()
{
	SetShaderMode(SHADER_MODE_DEFAULT);
	
	// �s�N�Z���V�F�[�_�[��ݒ�
	GetDeviceContext()->PSSetShader(g_PixelShaderSSAOBlur, NULL, 0);

	// �N���A�V�F�[�_�[���\�[�X
	ID3D11ShaderResourceView* null[] = { nullptr, nullptr };
	GetDeviceContext()->PSSetShaderResources(6, 1, null);

	// �r���[�|�[�g��ݒ�
	D3D11_VIEWPORT vp;
	vp.Width = (float)SCREEN_WIDTH;
	vp.Height = (float)SCREEN_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	GetDeviceContext()->RSSetViewports(1, &vp);

	// �����_�[�^�[�Q�b�g��ݒ�
	GetDeviceContext()->OMSetRenderTargets(1, &g_SSAOBlurTexRTV, NULL);

	// �����_�����O�^�[�Q�b�g���N���A
	float Clear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	GetDeviceContext()->ClearRenderTargetView(g_SSAOBlurTexRTV, Clear);

	// ���_�o�b�t�@��ݒ�
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

	// �����_�[�^�[�Q�b�g��ݒ�
	ID3D11RenderTargetView* nullRTV[] = { nullptr };
	GetDeviceContext()->OMSetRenderTargets(1, nullRTV, nullptr);

	// �V�F�[�_�[�Ƀe�N�X�`����ݒ肷��
	GetDeviceContext()->PSSetShaderResources(6, 1, &g_SSAOBlurTexSRV);

	SetShaderMode(SHADER_MODE_DEFAULT);
}

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

	case ViewPosMap:
		return g_PixelShaderViewPosMap;
		break;

	case INSTViewPosMap:
		return g_PixelShaderInstViewPosMap;
		break;
	}
}

ID3D11VertexShader* GetSSAOVertexShader(int pass)
{
	switch (pass)
	{
	case INSTNormalZMap:
		return g_SSAOInstVS;
		break;

	case INSTViewPosMap:
		return g_VertexShaderInstViewPosMap;
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

	case INSTViewPosMap:
		return g_InputLayoutInstViewPosMap;
		break;
	}
}

void SetSSAOConstant(void)
{
	XMMATRIX Tex = XMMATRIX(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);

	g_SSAOConstant.ViewToTex = XMMatrixTranspose(Tex);

	g_SSAOConstant.OcclusionFadeEnd = OCC_FADE_END;
	g_SSAOConstant.OcclusionFadeStart = OCC_FADE_START;
	g_SSAOConstant.OcclusionRadius = OCC_RADIUS;
	g_SSAOConstant.SurfaceEpsilon = SURFACE_EPSILON;
	
	g_SSAOConstant.FrustumCorners[0] = XMFLOAT4(-SCREEN_WIDTH / 2, -SCREEN_HEIGHT / 2, VIEW_FAR_Z, 0.0f);
	g_SSAOConstant.FrustumCorners[1] = XMFLOAT4(-SCREEN_WIDTH / 2, +SCREEN_HEIGHT / 2, VIEW_FAR_Z, 0.0f);
	g_SSAOConstant.FrustumCorners[2] = XMFLOAT4(+SCREEN_WIDTH / 2, +SCREEN_HEIGHT / 2, VIEW_FAR_Z, 0.0f);
	g_SSAOConstant.FrustumCorners[3] = XMFLOAT4(+SCREEN_WIDTH / 2, -SCREEN_HEIGHT / 2, VIEW_FAR_Z, 0.0f);

	GetDeviceContext()->UpdateSubresource(g_SSAOConstantBuffer, 0, NULL, &g_SSAOConstant, 0, 0);
}

void SetSSAOOffsetVectors(void)
{
	XMFLOAT4 mOffsets[14];
	// 8 �����̂̒��_
	mOffsets[0] = XMFLOAT4(+1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[1] = XMFLOAT4(-1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[2] = XMFLOAT4(-1.0f, +1.0f, +1.0f, 0.0f);
	mOffsets[3] = XMFLOAT4(+1.0f, -1.0f, -1.0f, 0.0f);

	mOffsets[4] = XMFLOAT4(+1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[5] = XMFLOAT4(-1.0f, -1.0f, +1.0f, 0.0f);

	mOffsets[6] = XMFLOAT4(-1.0f, +1.0f, -1.0f, 0.0f);
	mOffsets[7] = XMFLOAT4(+1.0f, -1.0f, +1.0f, 0.0f);

	// 6 �ʂ̒��S
	mOffsets[8] = XMFLOAT4(-1.0f, 0.0f, 0.0f, 0.0f);
	mOffsets[9] = XMFLOAT4(+1.0f, 0.0f, 0.0f, 0.0f);

	mOffsets[10] = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
	mOffsets[11] = XMFLOAT4(0.0f, +1.0f, 0.0f, 0.0f);

	mOffsets[12] = XMFLOAT4(0.0f, 0.0f, -1.0f, 0.0f);
	mOffsets[13] = XMFLOAT4(0.0f, 0.0f, +1.0f, 0.0f);

	for (int i = 0; i < 14; i++)
	{
		// 0.25����1.0�͈̔͂Ƀ����_���ɂ���
		float s = 0.25f + (float)(rand() % 512) / 511 * 0.75f;
		XMVECTOR v = s * XMVector4Normalize(XMLoadFloat4(&mOffsets[i]));
		XMStoreFloat4(&mOffsets[i], v);

		g_SSAOOffset.OffsetVectors[i] = mOffsets[i];
	}

	GetDeviceContext()->UpdateSubresource(g_SSAOOffsetBuffer, 0, NULL, &g_SSAOOffset, 0, 0);
}

static XMMATRIX InverseTranspose(CXMMATRIX M)
{
	XMMATRIX A = M;
	A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMVECTOR det = XMMatrixDeterminant(A);
	return XMMatrixTranspose(XMMatrixInverse(&det, A));
}
