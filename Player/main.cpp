#include "resource.h"
#include "MainWindow.h"
#include "Player.h"

static HINSTANCE g_hInst = NULL;
static Player*	 g_pPlayer = NULL;
static HMENU	 g_hMenu = NULL;
static HMENU	 g_hPopupMenu = NULL;
static HWND		 g_hTrackbar = NULL;


LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
VOID OnChar(HWND hwnd, TCHAR c);
void OnSize(HWND hwnd);
void OnOpenFile(HWND hWnd);
void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);
HWND WINAPI CreateTrackbar(HWND hwndDlg, UINT iMin, UINT iMax);


int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,_In_ int nCmdShow)
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE)))
	{
		MessageBox(NULL,
			_T("CoInitialize failed!"),
			_T("bakaPlayer"),
			NULL);
		return 1;
	}

	g_hInst = hInstance;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 3);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClass failed!"),
			_T("Error"),
			NULL);

		return 1;
	}
	
	HWND hWnd = CreateWindowEx(WS_EX_ACCEPTFILES, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("CreateWindow failed!"),
			_T("bakaPlayer"),
			NULL);

		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	g_hTrackbar = CreateTrackbar(hWnd, 0, 0);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (uMsg) {
	case WM_CREATE:
	{
		PROCESS_INFORMATION pi = { };
		STARTUPINFO si = { };
		si.cb = sizeof(si);
		si.wShowWindow = SW_HIDE;
		TCHAR cmd[] = _T("regsvr32.exe /s mp4demux.dll");
		DWORD res = CreateProcess(NULL, cmd, NULL, NULL, FALSE, FALSE, NULL, NULL, &si, &pi);
		res = WaitForSingleObject(pi.hProcess, 10000);
		if (res == WAIT_TIMEOUT)
		{
			MessageBox(hWnd, _T("Cannot register mp4 filter"), _T("Error"), MB_ICONERROR);
		}
		g_hMenu = CreateMenu();
		g_hPopupMenu = CreatePopupMenu();
		MenuInit(g_hMenu, g_hPopupMenu);
		SetMenu(hWnd, g_hMenu);
		g_pPlayer = new Player(hWnd);
		SetFocus(hWnd);
		return 0;
	}
	case WM_DROPFILES: 
	{
		TCHAR filename[MAX_PATH];
		DragQueryFile((HDROP)wParam, NULL, filename, MAX_PATH);
		HRESULT hr;
		hr = g_pPlayer->OpenFile(filename);
		InvalidateRect(hWnd, NULL, FALSE);
		if (SUCCEEDED(hr))
		{
			OnSize(hWnd);
		}
		else
		{
			MessageBox(hWnd, _T("Cannot open this file."), _T("Error"), MB_ICONERROR);
		}
		DragFinish((HDROP)wParam);
		SetFocus(hWnd);
	}
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_RIGHT:
		{
			LONGLONG pos = g_pPlayer->GetTime();
			g_pPlayer->SetPos(pos + 5);
			return 0;
		}
		case VK_LEFT:
		{
			LONGLONG pos = g_pPlayer->GetTime();
			g_pPlayer->SetPos(pos - 5);
			return 0;
		}
		}
		break;
	}
	case WM_CHAR:
	{
		OnChar(hWnd, TCHAR(wParam));
	}
	case WM_PAINT:
	{
		if (g_pPlayer != nullptr)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			g_pPlayer->UpdateVideoWindow(rc);
		}
		break;
	}
	case WM_DESTROY:
	{
		DestroyMenu(g_hMenu);
		DestroyMenu(g_hPopupMenu);
		DestroyWindow(g_hTrackbar);
		CoUninitialize();
		delete g_pPlayer;
		PROCESS_INFORMATION pi = { };
		STARTUPINFO si = { };
		si.cb = sizeof(si);
		si.wShowWindow = SW_HIDE;
		TCHAR cmd[] = _T("regsvr32.exe /s /u mp4demux.dll");
		DWORD res = CreateProcess(NULL, cmd, NULL, NULL, FALSE, FALSE, NULL, NULL, &si, &pi);
		res = WaitForSingleObject(pi.hProcess, 10000);
		if (res == WAIT_TIMEOUT)
		{
			MessageBox(hWnd, _T("Cannot deregister mp4 filter"), _T("Error"), MB_ICONERROR);
		}
		PostQuitMessage(0);
		return 0;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_TIMER:
		{
			if (g_hTrackbar == NULL)
				return 0;
			LONGLONG currentTime = g_pPlayer->GetTime();
			SendMessage(g_hTrackbar, TBM_SETPOS, TRUE, currentTime);
			return 0;
		}
		/*case IDT_TIMER1:
		{
			POINT p;
			GetCursorPos(&p);
			ScreenToClient(hWnd, &p);
			RECT rc;
			GetClientRect(hWnd, &rc);
			if ((p.x > rc.left && p.x < rc.right) && (p.y < rc.bottom && p.y > rc.bottom - TRACKBAR_SIZE))
			{
				if((g_hTrackbar == NULL))
					g_hTrackbar = CreateTrackbar(hWnd, 0, g_pPlayer->GetDuration());
				return 0;
			}
			else if (g_hTrackbar != NULL)
			{
				EnableWindow(g_hTrackbar, FALSE);
				DestroyWindow(g_hTrackbar);
				g_hTrackbar = NULL;
			}
			return 0;

		}*/
		}
		return 0;
	}
	case WM_MOUSEMOVE:
	{
		/*TRACKMOUSEEVENT tme = { 0 };
		DWORD dwPos = GetMessagePos();
		POINTS pts = MAKEPOINTS(dwPos);

		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_HOVER | TME_LEAVE | TME_NONCLIENT;
		tme.hwndTrack = hWnd;
		tme.dwHoverTime = 10;

		g_bMouseTrack = TrackMouseEvent(&tme);
		

		RECT rc;
		GetClientRect(hWnd, &rc);
		if ((GET_Y_LPARAM(lParam) > rc.bottom - TRACKBAR_SIZE) && (g_hTrackbar == NULL))
		{
			g_hTrackbar = CreateTrackbar(hWnd, 0, g_pPlayer->GetDuration());
		}
		else if (g_hTrackbar != NULL)
		{
			EnableWindow(g_hTrackbar, FALSE);
			DestroyWindow(g_hTrackbar);
			g_hTrackbar = NULL;
		}
*/
		return 0;
	}
	case WM_MOUSELEAVE:
	{
		/*EnableWindow(g_hTrackbar, FALSE);
		DestroyWindow(g_hTrackbar);
		g_hTrackbar = NULL;*/

		return 0;
	}
	case WM_MOUSEHOVER:
	{
		return 0;
	}
	case WM_SIZE:
	{	
		OnSize(hWnd);
		break;
	}
	case WM_MOVE:
	{		
		if (g_pPlayer != nullptr)
			g_pPlayer->HandleMoveEvent((OAHWND)hWnd, uMsg, wParam, lParam);
		break;
	}
	case WM_COMMAND:
	{
		switch (wParam)
		{
		case MENU_OPEN:
		{
			OnOpenFile(hWnd);
			return 0;
		}
		case MENU_ASPECT_RATIO:
		{
			MENUITEMINFO mii;
			mii.fMask = MIIM_STATE;
			mii.cbSize = sizeof(MENUITEMINFO);
			GetMenuItemInfo(g_hPopupMenu, MENU_ASPECT_RATIO, FALSE, &mii);
			if (mii.fState & MFS_CHECKED)
			{
				mii.fState = MFS_UNCHECKED;
			}
			else
			{
				mii.fState = MFS_CHECKED;
			}

			SetMenuItemInfo(g_hPopupMenu, MENU_ASPECT_RATIO, FALSE, &mii);

			DrawMenuBar(hWnd);

			g_pPlayer->ChangeSaveAspectRatio();

			return 0;
		}
		case MENU_ABOUT:
		{
			MessageBox(hWnd, _T("bakaPlayer\n\rby Anton Kourganov\n\r2019"), _T("About"), MB_ICONQUESTION | MB_TOPMOST);
			return 0;
		}
		}
	}
	case WM_GRAPH_EVENT:
	{
		if(g_pPlayer != nullptr)
			g_pPlayer->HandleGraphEvent(OnGraphEvent);
		return 0;
	}
	case WM_HSCROLL:
	{
		KillTimer(hWnd, IDT_TIMER);
		g_pPlayer->Pause();
		LONGLONG pos = SendMessage(g_hTrackbar, TBM_GETPOS, 0, 0);
		g_pPlayer->SetPos(pos);
		g_pPlayer->Play();
		SetTimer(hWnd, IDT_TIMER, TICK_FREQ, (TIMERPROC)NULL);
		SetFocus(hWnd);
	}
	}
	 
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void OnChar(HWND hwnd, TCHAR c)
{
	switch (c)
	{
	case _T(' '):
		if (g_pPlayer->State() == STATE_RUNNING)
		{
			g_pPlayer->Pause();
		}
		else
		{
			g_pPlayer->Play();
			KillTimer(hwnd, IDT_TIMER);
			SetTimer(hwnd, IDT_TIMER, TICK_FREQ, (TIMERPROC)NULL);
		}
		break;
	}
}


void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2)
{
	switch (evCode)
	{
	case EC_COMPLETE:
	{
		KillTimer(hwnd, IDT_TIMER);
		LONGLONG maxpos = SendMessage(g_hTrackbar, TBM_GETRANGEMAX, 0, 0);
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, maxpos);
		g_pPlayer->Stop();
		break;
	}
	case EC_USERABORT:
	{
		KillTimer(hwnd, IDT_TIMER);
		g_pPlayer->Stop();
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, g_pPlayer->GetDuration());
		break;
	}
	case EC_ERRORABORT:
	{
		KillTimer(hwnd, IDT_TIMER);
		g_pPlayer->Stop();
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, g_pPlayer->GetDuration());
		MessageBox(hwnd, _T("Playback error."), _T("Error"), MB_ICONERROR);
		break;
	}
	}
}


void OnSize(HWND hwnd)
{
	if (g_pPlayer)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		g_pPlayer->UpdateVideoWindow(rc);
	}
	if (g_hTrackbar != NULL)
	{
		EnableWindow(g_hTrackbar, FALSE);
		DestroyWindow(g_hTrackbar);
		g_hTrackbar = NULL;
		g_hTrackbar = CreateTrackbar(hwnd, 0, g_pPlayer->GetDuration());
	}
}


void OnOpenFile(HWND hWnd)
{
	TCHAR path[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, path);

	OPENFILENAME ofn = { };
	TCHAR szFileName[MAX_PATH];
	szFileName[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrFilter = _T("All (*.*)/0*.*/0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		HRESULT hr;
		hr = g_pPlayer->OpenFile(ofn.lpstrFile);
		InvalidateRect(hWnd, NULL, FALSE);
		if (SUCCEEDED(hr))
		{
			OnSize(hWnd);
		}
		else
		{
			MessageBox(hWnd, _T("Cannot open this file."), _T("Error"), MB_ICONERROR);
		}
	}

	SetCurrentDirectory(path);
	SetFocus(hWnd);
}


HWND WINAPI CreateTrackbar(HWND hwndDlg, UINT iMin, UINT iMax)
{
	INITCOMMONCONTROLSEX iccs;
	iccs.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccs.dwICC = ICC_BAR_CLASSES;

	InitCommonControlsEx(&iccs);

	RECT rc;
	GetClientRect(hwndDlg, &rc);

	HWND hTrackbar = CreateWindowEx(
		WS_EX_TOPMOST,         
		TRACKBAR_CLASS,
		_T("Trackbar Control"),
		WS_CHILD |
		WS_VISIBLE |
		TBS_NOTICKS,
		rc.left, rc.bottom-TRACKBAR_SIZE,
		rc.right, TRACKBAR_SIZE,
		hwndDlg,                     
		(HMENU)ID_TRACKBAR,
		g_hInst,
		NULL);

	SendMessage(hTrackbar, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));

	SendMessage(hTrackbar, TBM_SETPAGESIZE, 0, (LPARAM)1);
	if (!g_pPlayer)
	{
		SendMessage(hTrackbar, TBM_SETPOS, (WPARAM)TRUE, 0);
		return hTrackbar;
	}

	if (g_pPlayer->State() == STATE_STOPPED)
	{
		LONGLONG maxpos = SendMessage(g_hTrackbar, TBM_GETRANGEMAX, 0, 0);
		SendMessage(hTrackbar, TBM_SETPOS, (WPARAM)TRUE, maxpos);
	}
	else
		SendMessage(hTrackbar, TBM_SETPOS, (WPARAM)TRUE, g_pPlayer->GetTime());           


	return hTrackbar;

}
