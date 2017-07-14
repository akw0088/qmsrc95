#include <windows.h>
#include <time.h>

typedef unsigned short fixang;

#define ANG_STEP  0x0080
#define VEL_STEP  0.5

typedef struct
{
	float x, y, z;
} vector;

typedef struct
{
	fixang tx, ty, tz;
} angvec;



char maps[9][64] = { "maps/start.bsp",
"maps/e1m1.bsp",
"maps/e1m2.bsp",
"maps/e1m3.bsp",
"maps/e1m4.bsp",
"maps/e1m5.bsp",
"maps/e1m6.bsp",
"maps/e1m7.bsp",
"maps/e1m8.bsp" };


// spawn location
vector start_loc[9] = {
	{ 500, 240, 100 },  // start
	{ 476, -112, 50 },  // e1m1: the Slipgate Complex
	{ 500, 240, 100 },  // e1m2: Castle of the Damned
	{ 540, 240, 272 },  // e1m3 the Necropolis
	{ 500, 266, 928 },  // e1m4: the Grisly Grotto
	{ 500, 240, 100 },  // e1m5: Gloom Keep
	{ 500, 240, 100 },  // e1m6: The Door To Chthon
	{ 500, 240, 100 },  // e1m7: Chthon
	{ 560, 440, -526 }, // e1m8 Ziggurat Vertigo
};



int	cxClient = 1, cyClient = 1;
int *pixel = NULL;

extern vector cam_loc, cam_vel, new_loc;
extern angvec cam_ang, cam_angvel;

#define WM_TICK 1

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int dosmain(char *map);


DWORD WINAPI start_thread(LPVOID lpParam)
{
	dosmain((char *)lpParam);
	return 0;
}

void draw_pixels(HDC hdc, HDC hdcMem, int width, int height)
{
	HBITMAP hBitmap, hOldBitmap;

	hBitmap = CreateCompatibleBitmap(hdc, width, height);
	SetBitmapBits(hBitmap, sizeof(int) * width * height, pixel);
	hdcMem = CreateCompatibleDC(hdc);
	hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
	// This scaling is a little strange because Stretch maintains aspect ratios
	StretchBlt(hdc, 0, 0, cxClient, cyClient, hdcMem, 0, 0, width, height, SRCCOPY);
	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);
	DeleteObject(hBitmap);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("qmsrc95");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(NULL_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("Program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}



	hwnd = CreateWindow(szAppName, TEXT("qmsrc95"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT	rect = { 0, 0, 1, 1 };
	static HDC	hdcMem;
	static int	old_style = 0;
	static int	new_style = WS_CHILD | WS_VISIBLE;
	static int	xres, yres;
	HDC		hdc;
	PAINTSTRUCT	ps;
	static HANDLE dosthread;
	static int r;

	switch (message)
	{
	case WM_CREATE:
	{
		HMONITOR hmon;
		MONITORINFO mi = { sizeof(MONITORINFO) };

		hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		GetMonitorInfo(hmon, &mi);

		xres = abs(mi.rcMonitor.right - mi.rcMonitor.left);
		yres = abs(mi.rcMonitor.bottom - mi.rcMonitor.top);

		pixel = (int *)malloc(sizeof(int) * xres * yres);
		SetTimer(hwnd, WM_TICK, 16, NULL);
		
		srand((unsigned int)time(0));
		r = rand() % 10;


		cam_loc = start_loc[r];
		dosthread = CreateThread(NULL, 0, start_thread, maps[r], 0, NULL);

		SetWindowPos(hwnd, 0, 5, 5, 3 * 320, 3 * 200, 0);


		return 0;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_F1)
		{
			old_style = SetWindowLong(hwnd, GWL_STYLE, new_style);
			new_style = old_style;
			SetWindowPos(hwnd, HWND_TOP, 0, 0, xres, yres, 0);
		}


		SuspendThread(dosthread);

		switch (wParam)
		{
		case 'V': //lookdown
			cam_ang.tx += 4 * ANG_STEP;
			break;
		case 'R': //lookup
			cam_ang.tx -= 4 * ANG_STEP;
			break;
		case 'Z': //roll
			cam_ang.ty += 4 * ANG_STEP;
			break;
		case 'C':// roll
			cam_ang.ty -= 4 * ANG_STEP;
			break;
		case 'Q': // rotate right
			cam_ang.tz += 4 * ANG_STEP;
			break;
		case 'E': // rotate left
			cam_ang.tz -= 4 * ANG_STEP;
			break;
		case 'D': // strafe right
			cam_loc.x += 4 * VEL_STEP;
			break;
		case 'A': // strafe left
			cam_loc.x -= 4 * VEL_STEP;
			break;
		case '1': // move down
			cam_loc.z -= 4 * VEL_STEP;
			break;
		case '3': // move up
			cam_loc.z += 4 * VEL_STEP;
			break;
		case 'S': // move back
			cam_loc.y -= 4 * VEL_STEP;
			break;
		case 'W':// forward
			cam_loc.y += 4 * VEL_STEP;
			break;
		}

		ResumeThread(dosthread);
	}
	case WM_TIMER:
	{
		int width = cxClient;
		int height = cyClient;
		InvalidateRect(hwnd, &rect, FALSE);
		return 0;
	}
	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		rect.right = cxClient;
		rect.bottom = cyClient;
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		draw_pixels(hdc, hdcMem, 320, 200);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}