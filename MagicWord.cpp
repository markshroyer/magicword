#include "stdafx.h"
#include "MagicWord.h"
#include "ImageResource.h"
#include "LogString.h"

#define MAX_LOADSTRING 100
#define WINDOW_SIZE 480
#define TIMER_ARM 1

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
ImageResource* nedryBody;
ImageResource* nedryFace;
ImageResource* nedryArm;

// Forward declarations:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void ResizeClient(HWND, int, int);
void Draw(HDC, int, int);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	// We're using GDI+ to make things easier (and for higher quality)...
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MAGICWORD, szWindowClass, MAX_LOADSTRING);

	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAGICWORD));
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	Gdiplus::GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAGICWORD));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MAGICWORD);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MAGICWORD));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   hInst = hInstance;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, WINDOW_SIZE, WINDOW_SIZE, NULL, NULL, hInstance, NULL);

   if ( ! hWnd ) return FALSE;

   ResizeClient(hWnd, WINDOW_SIZE, WINDOW_SIZE);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HDC hdcBuffer;
	HBITMAP hbmpBuffer, hbmpOld;
	static int cxClient, cyClient;
	static int cxSource, cySource;

	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_CREATE:
		nedryBody = new ImageResource(MAKEINTRESOURCE(IDR_NEDRY_BODY));
		nedryFace = new ImageResource(MAKEINTRESOURCE(IDR_NEDRY_FACE));
		nedryArm  = new ImageResource(MAKEINTRESOURCE(IDR_NEDRY_ARM));

		SetTimer(hWnd, TIMER_ARM, 33, NULL);

		PlaySound(MAKEINTRESOURCE(IDR_MAGICWRD), hInst, SND_RESOURCE | SND_ASYNC | SND_LOOP);

		break;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		break;

	case WM_TIMER:
		switch (wParam) {
		case TIMER_ARM:
			InvalidateRgn(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
			break;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		// Double buffering to present animation flicker
		hdcBuffer = CreateCompatibleDC(hdc);
		hbmpBuffer = CreateCompatibleBitmap(hdc, cxClient, cyClient);
		hbmpOld = (HBITMAP) SelectObject(hdcBuffer, hbmpBuffer);

		Draw(hdcBuffer, cxClient, cyClient);
		BitBlt(hdc, 0, 0, cxClient, cyClient, hdcBuffer, 0, 0, SRCCOPY);

		SelectObject(hdcBuffer, hbmpOld);
		DeleteObject(hbmpBuffer);
		DeleteDC(hdcBuffer);

		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		KillTimer(hWnd, TIMER_ARM);
		delete nedryBody;
		delete nedryFace;
		delete nedryArm;
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

// Resize the window to result in a client area of exactly the specified dimensions
void ResizeClient(HWND hWnd, int width, int height)
{
	RECT rectClient, rectWindow;
	POINT ptDiff;
	GetClientRect(hWnd, &rectClient);
	GetWindowRect(hWnd, &rectWindow);
	ptDiff.x = width - rectClient.right;
	ptDiff.y = height - rectClient.bottom;

	MoveWindow(hWnd,
				rectWindow.left,
				rectWindow.top,
				(rectWindow.right - rectWindow.left) + ptDiff.x,
				(rectWindow.bottom - rectWindow.top) + ptDiff.y,
				TRUE);
}

// Draw Dennis Nedry's winsome mug
void Draw(HDC hdc, int width, int height)
{
	Gdiplus::Matrix affineTransform;
	Gdiplus::Status result;
	FILETIME ft;

	Gdiplus::Graphics graphics(hdc);
	graphics.Clear(Gdiplus::Color(0xff, 0xff, 0xff, 0xff));

	Gdiplus::Point ptNedryBody(width / 2 - 100, height / 2 - 60);
	graphics.DrawImage(nedryBody->lpImage, ptNedryBody);

	Gdiplus::Point ptNedryFace(width / 2 - 54, height / 2 - 180);
	graphics.DrawImage(nedryFace->lpImage, ptNedryFace);

	GetSystemTimeAsFileTime(&ft);
	double angle = 15 * cos( ( (double) ft.dwLowDateTime ) / 1000000 );

	Gdiplus::PointF ptNedryArm(width / 2 + 50, height / 2 - 100);
	Gdiplus::PointF ptNedryArmAxisOffset(13, 108);
	affineTransform.RotateAt((int) angle, ptNedryArm + ptNedryArmAxisOffset);
	graphics.SetTransform(&affineTransform);
	graphics.DrawImage(nedryArm->lpImage, ptNedryArm);
	graphics.ResetTransform();
}