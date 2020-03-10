#include<Windows.h>
#include<d3dx11.h>
#include<dxerr.h>

#define TITLE TEXT ("Test")
#define WIDTH	640
#define	HEIGHT	480

#define SAFE_RELEASE(x) {if(x){(x)->Release(); (x)=NULL;}}

#pragma comment(lib,"d3d11.lib")

/* �f�o�b�O�łƃ����[�X�łł͓ǂݍ���lib���Ⴄ */
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib,"d3dx11d.lib")
#else
#pragma comment(lib,"d3dx11.lib")
#endif

#pragma comment(lib,"dxerr.lib")
#pragma comment(lib,"winmm.lib")

D3D_FEATURE_LEVEL g_pFeatureLevels[] = { D3D_FEATURE_LEVEL_11_0,D3D_FEATURE_LEVEL_10_1,D3D_FEATURE_LEVEL_10_0 };
UINT g_FeatureLevels = 3;
D3D_FEATURE_LEVEL g_FeatureLevelsSupported;

/* �C���^�[�t�F�C�X */
ID3D11Device*				g_pD3DDevice = NULL;				//<<�@�f�o�C�X
ID3D11DeviceContext*		g_pImmediateContext = NULL;			//<<�@�f�o�C�X�E�R���e�L�X�g
IDXGISwapChain*				g_pSwapChain = NULL;				//<<�@�X���b�v�E�`�F�C��
ID3D11RenderTargetView*		g_pRenderTargetView = NULL;			//<<�@�`��^�[�Q�b�g�E�r���[
D3D11_VIEWPORT				g_ViewPort[1];						//<<�@�r���[�|�[�g�̐ݒ�
ID3D11Texture2D*			g_pDepthStencil;					//<<�@�[�x/�X�e���V���E�e�N�X�`�����󂯎��ϐ�
ID3D11DepthStencilView*		g_pDepthStencilView;				//<<�@�X�e���V���E�r���[

#if defined(DEBUG) || defined(_DEBUG)
UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
UINT createDeviceFlags = 0;
#endif

/* ���������� */
bool CreateDirect3D(HWND _hwnd)
{
	/* �X���b�v�`�F�C���̐ݒ� */
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 640;
	sd.BufferDesc.Height = 480;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;		//<<���q
	sd.BufferDesc.RefreshRate.Denominator = 1;		//<<����
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		//<<�o�b�N�o�b�t�@�̎g�p�@
	sd.OutputWindow = _hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	/* �f�o�C�X�ƃX���b�v�`�F�C���̍쐬 */
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		g_pFeatureLevels,
		g_FeatureLevels,
		D3D11_SDK_VERSION,
		&sd,
		&g_pSwapChain,
		&g_pD3DDevice,
		&g_FeatureLevelsSupported,
		&g_pImmediateContext);
	if (FAILED(hr))
	{
		/* WARP�f�o�C�X���쐬 */
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			D3D_DRIVER_TYPE_WARP,
			NULL,
			createDeviceFlags,
			g_pFeatureLevels,
			g_FeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&g_pSwapChain,
			&g_pD3DDevice,
			&g_FeatureLevelsSupported,
			&g_pImmediateContext);
		if (FAILED(hr))
		{
			/* ���t�@�����X�f�o�C�X�̍쐬 */
			hr = D3D11CreateDeviceAndSwapChain(
				NULL,
				D3D_DRIVER_TYPE_REFERENCE,
				NULL,
				createDeviceFlags,
				g_pFeatureLevels,
				g_FeatureLevels,
				D3D11_SDK_VERSION,
				&sd,
				&g_pSwapChain,
				&g_pD3DDevice,
				&g_FeatureLevelsSupported,
				&g_pImmediateContext);
			if (FAILED(hr)) return false;
		}
	}

	/* �o�b�N�o�b�t�@���擾 */
	ID3D11Texture2D *pBackBuffer;
	HRESULT bhr = g_pSwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);
	if (FAILED(bhr)) return false;

	/* �o�b�N�o�b�t�@�̕`��^�[�Q�b�g�E�r���[����� */
	bhr = g_pD3DDevice->CreateRenderTargetView(
		pBackBuffer,
		NULL,
		&g_pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	if (FAILED(bhr)) return false;

	/* �`��^�[�Q�b�g�E�r���[���o�̓}�l�[�W���[�̕`��^�[�Q�b�g�Ƃ��Đݒ� */
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		NULL);

	/* �r���[�|�[�g�̐ݒ� */
	g_ViewPort[0].TopLeftX = 0.0f;
	g_ViewPort[0].TopLeftY = 0.0f;
	g_ViewPort[0].Width = 640.0f;
	g_ViewPort[0].Height = 480.0f;
	g_ViewPort[0].MinDepth = 0.0f;
	g_ViewPort[0].MaxDepth = 1.0f;

	/* ���X�^���C�U�Ƀr���[�|�[�g��ݒ� */
	g_pImmediateContext->RSSetViewports(1, g_ViewPort);

	/* �X�e���V���E�e�N�X�`���̍쐬 */

	/* �[�x�X�e���V���o�b�t�@�̐ݒ� */
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = 640;
	descDepth.Height = 480;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	bhr = g_pD3DDevice->CreateTexture2D(
		&descDepth,
		NULL,
		&g_pDepthStencil);
	if (FAILED(bhr)) return 0;

	/* �X�e���V���E�r���[�̍쐬 */
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Flags = 0;
	descDSV.Texture2D.MipSlice = 0;
	bhr = g_pD3DDevice->CreateDepthStencilView(
		g_pDepthStencil,
		&descDSV,
		&g_pDepthStencilView);
	if (FAILED(bhr)) return 0;

	/* �X�e���V���E�r���[�̎g�p */
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		g_pDepthStencilView);

	/* �`��^�[�Q�b�g�̃N���A */
	float ClearColor[4] = { 0.0f,0.125f,0.3f,1.0f };
	g_pImmediateContext->ClearRenderTargetView(
		g_pRenderTargetView,
		ClearColor);

	/* �X�e���V���l�̃N���A */
	g_pImmediateContext->ClearDepthStencilView(
		g_pDepthStencilView,
		D3D11_CLEAR_DEPTH,
		1.0f,
		0);


	return true;
}

/* ������� */
void ReleaseDevice()
{
	if (g_pImmediateContext) g_pImmediateContext->ClearState();

	SAFE_RELEASE(g_pRenderTargetView);
	SAFE_RELEASE(g_pSwapChain);
	SAFE_RELEASE(g_pImmediateContext);
	SAFE_RELEASE(g_pD3DDevice);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_KEYDOWN:
		switch (wp)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInsta, LPSTR szStr, INT iCmdShow)
{
	WNDCLASS Wndclass;
	HWND hWnd;
	MSG msg;

	//�E�B���h�E�̐ݒ�
	Wndclass.style = CS_HREDRAW | CS_VREDRAW;
	Wndclass.lpfnWndProc = WndProc;
	Wndclass.cbClsExtra = 0;
	Wndclass.cbWndExtra = 0;
	Wndclass.hInstance = hInstance;
	Wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	Wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	Wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	Wndclass.lpszMenuName = NULL;
	Wndclass.lpszClassName = TITLE;

	RegisterClass(&Wndclass);

	hWnd = CreateWindow(
		TITLE,
		TITLE,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WIDTH,
		HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (!hWnd)return 0;

	/* ������ */
	if (!CreateDirect3D(hWnd)) return 0;

	/* ���Ԃ̌v�� */
	DWORD CurrentTime = timeGetTime();
	DWORD PrevTime = timeGetTime();

	timeBeginPeriod(1);
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		Sleep(1);
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			CurrentTime = timeGetTime();

			if (PrevTime - CurrentTime >= 1000 / 60)
			{
				/* �f�o�C�X�̏������� */
				HRESULT hr;
				hr = g_pD3DDevice->GetDeviceRemovedReason();
				switch (hr)
				{
				case S_OK:
					break;	//<<�@����
				case DXGI_ERROR_DEVICE_HUNG:
				case DXGI_ERROR_DEVICE_RESET:
					ReleaseDevice();
					if (!CreateDirect3D(hWnd)) return 0;
					break;
				case DXGI_ERROR_DEVICE_REMOVED:
				case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				case DXGI_ERROR_INVALID_CALL:
				default:
					return 0;	//<<�@�ǂ����悤���Ȃ��̂ŃA�v���P�[�V�������I��
				}


				/* �`�� */
				g_pSwapChain->Present(0, 0);
				CurrentTime = PrevTime;
			}
		}
	}
	timeEndPeriod(1);

	ReleaseDevice();

	return (int)msg.wParam;
}