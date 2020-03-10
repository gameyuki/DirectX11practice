#include<Windows.h>
#include<d3dx11.h>
#include<dxerr.h>

#define TITLE TEXT ("Test")
#define WIDTH	640
#define	HEIGHT	480

#define SAFE_RELEASE(x) {if(x){(x)->Release(); (x)=NULL;}}

#pragma comment(lib,"d3d11.lib")

/* デバッグ版とリリース版では読み込むlibが違う */
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

/* インターフェイス */
ID3D11Device*				g_pD3DDevice = NULL;				//<<　デバイス
ID3D11DeviceContext*		g_pImmediateContext = NULL;			//<<　デバイス・コンテキスト
IDXGISwapChain*				g_pSwapChain = NULL;				//<<　スワップ・チェイン
ID3D11RenderTargetView*		g_pRenderTargetView = NULL;			//<<　描画ターゲット・ビュー
D3D11_VIEWPORT				g_ViewPort[1];						//<<　ビューポートの設定
ID3D11Texture2D*			g_pDepthStencil;					//<<　深度/ステンシル・テクスチャを受け取る変数
ID3D11DepthStencilView*		g_pDepthStencilView;				//<<　ステンシル・ビュー

#if defined(DEBUG) || defined(_DEBUG)
UINT createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
UINT createDeviceFlags = 0;
#endif

/* 初期化処理 */
bool CreateDirect3D(HWND _hwnd)
{
	/* スワップチェインの設定 */
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = 640;
	sd.BufferDesc.Height = 480;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;		//<<分子
	sd.BufferDesc.RefreshRate.Denominator = 1;		//<<分母
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;		//<<バックバッファの使用法
	sd.OutputWindow = _hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	/* デバイスとスワップチェインの作成 */
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
		/* WARPデバイスを作成 */
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
			/* リファレンスデバイスの作成 */
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

	/* バックバッファを取得 */
	ID3D11Texture2D *pBackBuffer;
	HRESULT bhr = g_pSwapChain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(LPVOID*)&pBackBuffer);
	if (FAILED(bhr)) return false;

	/* バックバッファの描画ターゲット・ビューを作る */
	bhr = g_pD3DDevice->CreateRenderTargetView(
		pBackBuffer,
		NULL,
		&g_pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
	if (FAILED(bhr)) return false;

	/* 描画ターゲット・ビューを出力マネージャーの描画ターゲットとして設定 */
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		NULL);

	/* ビューポートの設定 */
	g_ViewPort[0].TopLeftX = 0.0f;
	g_ViewPort[0].TopLeftY = 0.0f;
	g_ViewPort[0].Width = 640.0f;
	g_ViewPort[0].Height = 480.0f;
	g_ViewPort[0].MinDepth = 0.0f;
	g_ViewPort[0].MaxDepth = 1.0f;

	/* ラスタライザにビューポートを設定 */
	g_pImmediateContext->RSSetViewports(1, g_ViewPort);

	/* ステンシル・テクスチャの作成 */

	/* 深度ステンシルバッファの設定 */
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

	/* ステンシル・ビューの作成 */
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

	/* ステンシル・ビューの使用 */
	g_pImmediateContext->OMSetRenderTargets(
		1,
		&g_pRenderTargetView,
		g_pDepthStencilView);

	/* 描画ターゲットのクリア */
	float ClearColor[4] = { 0.0f,0.125f,0.3f,1.0f };
	g_pImmediateContext->ClearRenderTargetView(
		g_pRenderTargetView,
		ClearColor);

	/* ステンシル値のクリア */
	g_pImmediateContext->ClearDepthStencilView(
		g_pDepthStencilView,
		D3D11_CLEAR_DEPTH,
		1.0f,
		0);


	return true;
}

/* 解放処理 */
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

	//ウィンドウの設定
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

	/* 初期化 */
	if (!CreateDirect3D(hWnd)) return 0;

	/* 時間の計測 */
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
				/* デバイスの消失処理 */
				HRESULT hr;
				hr = g_pD3DDevice->GetDeviceRemovedReason();
				switch (hr)
				{
				case S_OK:
					break;	//<<　正常
				case DXGI_ERROR_DEVICE_HUNG:
				case DXGI_ERROR_DEVICE_RESET:
					ReleaseDevice();
					if (!CreateDirect3D(hWnd)) return 0;
					break;
				case DXGI_ERROR_DEVICE_REMOVED:
				case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
				case DXGI_ERROR_INVALID_CALL:
				default:
					return 0;	//<<　どうしようもないのでアプリケーションを終了
				}


				/* 描画 */
				g_pSwapChain->Present(0, 0);
				CurrentTime = PrevTime;
			}
		}
	}
	timeEndPeriod(1);

	ReleaseDevice();

	return (int)msg.wParam;
}