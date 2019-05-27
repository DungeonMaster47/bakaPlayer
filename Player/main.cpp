#include "resource.h"
#include "MainWindow.h"
#include "Player.h"

static HINSTANCE g_hInst = NULL;
static Player*	 g_pPlayer = NULL;
static HMENU	 g_hMenu = NULL;
static HMENU	 g_hPopupMenu = NULL;
static HWND		 g_hTrackbar = NULL;
static HWND		 g_hVolumebar = NULL;
static HWND		 g_hBackButton = NULL;
static HWND		 g_hPlayButton = NULL;
static HWND		 g_hStopButton = NULL;
static HWND		 g_hForwardButton = NULL;
static HWND		 g_hLabel = NULL;
static HWND		 g_hVolume = NULL;


LRESULT CALLBACK WndProc(_In_ HWND hwnd, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
VOID OnChar(HWND hwnd, TCHAR c);
void OnSize(HWND hwnd);
void OnOpenFile(HWND hWnd);
void CALLBACK OnGraphEvent(HWND hwnd, long evCode, LONG_PTR param1, LONG_PTR param2);
void CreateControlBar(HWND hwndDlg, UINT iMin, UINT iMax);
void DestroyControlBar();
void PlayPause(HWND hwnd);
void Stop(HWND hwnd);
void FileOpen(HWND hwnd, TCHAR* szFileName);
void CMDStart(HWND hwnd);


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

	CreateControlBar(hWnd, 0, 0);

	CMDStart(hWnd);

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
		TCHAR szFileName[MAX_PATH];
		DragQueryFile((HDROP)wParam, NULL, szFileName, MAX_PATH);
		FileOpen(hWnd, szFileName);
		DragFinish((HDROP)wParam);
		SetFocus(hWnd);
		return 0;
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
		return 0;
	}
	case WM_LBUTTONUP:
	{
		PlayPause(hWnd);
		return 0;
	}
	case WM_PAINT:
	{
		if (g_pPlayer != nullptr)
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			rc.bottom -= TRACKBAR_HEIGHT;
			g_pPlayer->UpdateVideoWindow(rc);
		}
		break;
	}
	case WM_DESTROY:
	{
		DestroyMenu(g_hMenu);
		DestroyMenu(g_hPopupMenu);
		DestroyControlBar();
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
			TCHAR szNewTitle[64];
			size_t duration = g_pPlayer->GetDuration();
			size_t time	    = g_pPlayer->GetTime();
			StringCbPrintf(szNewTitle, sizeof(szNewTitle), _T("%s - %d:%02d/%d:%02d"), szTitle, time/60, time%60, duration/60, duration%60);
			SetWindowText(hWnd, szNewTitle);
			StringCbPrintf(szNewTitle, sizeof(szNewTitle), _T("%d:%02d/%d:%02d"), time / 60, time % 60, duration / 60, duration % 60);
			SetWindowText(g_hLabel, szNewTitle);

			if (g_hTrackbar == NULL)
				CreateControlBar(hWnd, 0, g_pPlayer->GetDuration());
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
			if ((p.x > rc.left && p.x < rc.right) && (p.y < rc.bottom && p.y > rc.bottom - TRACKBAR_HEIGHT))
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
		break;
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
		if ((GET_Y_LPARAM(lParam) > rc.bottom - TRACKBAR_HEIGHT) && (g_hTrackbar == NULL))
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
		break;
	}
	case WM_MOUSELEAVE:
	{
		/*EnableWindow(g_hTrackbar, FALSE);
		DestroyWindow(g_hTrackbar);
		g_hTrackbar = NULL;*/

		break;
	}
	case WM_MOUSEHOVER:
	{
		break;
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
		if (LOWORD(wParam) == NULL)
			return 0;
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case ID_BACKBUTTON:
			{
				LONGLONG pos = g_pPlayer->GetTime();
				g_pPlayer->SetPos(pos - 5);
				return 0;
			}
			case ID_PLAYBUTTON:
			{
				PlayPause(hWnd);
				return 0;
			}
			case ID_STOPBUTTON:
			{
				Stop(hWnd);
				return 0;
			}
			case ID_FORWARDBUTTON:
			{
				LONGLONG pos = g_pPlayer->GetTime();
				g_pPlayer->SetPos(pos + 5);
				return 0;
			}
			}
		}
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
		if (GetDlgCtrlID((HWND)lParam) == ID_TRACKBAR)
		{
			if (g_pPlayer->State() == STATE_STOPPED)
				break;
			KillTimer(hWnd, IDT_TIMER);
			g_pPlayer->Pause();
			LONGLONG pos = SendMessage(g_hTrackbar, TBM_GETPOS, 0, 0);
			g_pPlayer->SetPos(pos);
			g_pPlayer->Play();
			SetTimer(hWnd, IDT_TIMER, TICK_FREQ, (TIMERPROC)NULL);
			SetFocus(hWnd);
		}
		else if ((GetDlgCtrlID((HWND)lParam) == ID_VOLUMEBAR))
		{
			g_pPlayer->SetVolumeLevel(SendMessage(g_hVolumebar, TBM_GETPOS, 0, 0));
			SetFocus(hWnd);
		}
		return 0;
	}
	}
	 
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void OnChar(HWND hwnd, TCHAR c)
{
	switch (c)
	{
	case _T(' '):
	{
		PlayPause(hwnd);
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
		Stop(hwnd);
		break;
	}
	case EC_USERABORT:
	{
		Stop(hwnd);
		break;
	}
	case EC_ERRORABORT:
	{
		Stop(hwnd);
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
		
		rc.bottom -= TRACKBAR_HEIGHT;

		g_pPlayer->UpdateVideoWindow(rc);

		CreateControlBar(hwnd, 0, g_pPlayer->GetDuration());
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
	ofn.lpstrFilter = _T("All\0*.*\0\0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		FileOpen(hWnd, ofn.lpstrFile);
	}

	SetCurrentDirectory(path);
	SetFocus(hWnd);
}


void CreateControlBar(HWND hwndDlg, UINT iMin, UINT iMax)
{
	DestroyControlBar();

	INITCOMMONCONTROLSEX iccs;
	iccs.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccs.dwICC = ICC_BAR_CLASSES;

	InitCommonControlsEx(&iccs);

	RECT rc;
	GetClientRect(hwndDlg, &rc);

	g_hTrackbar = CreateWindowEx(
		0,         
		TRACKBAR_CLASS,
		_T("Trackbar Control"),
		WS_CHILD |
		WS_VISIBLE |
		TBS_NOTICKS,
		rc.left, rc.bottom-TRACKBAR_HEIGHT,
		rc.right, TRACKBAR_HEIGHT-VOLUMEBAR_HEIGHT,
		hwndDlg,                     
		(HMENU)ID_TRACKBAR,
		g_hInst,
		NULL);

	SendMessage(g_hTrackbar, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));

	SendMessage(g_hTrackbar, TBM_SETPAGESIZE, 0, (LPARAM)1);

	if (!g_pPlayer)
	{
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, 0);
	} else if (g_pPlayer->State() == STATE_STOPPED)
	{
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, 0);
	}
	else
		SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, g_pPlayer->GetTime());

	g_hVolumebar = CreateWindowEx(
		WS_EX_TOPMOST,
		TRACKBAR_CLASS,
		_T("Volume Control"),
		WS_CHILD |
		WS_VISIBLE |
		TBS_NOTICKS |
		TBS_TOOLTIPS,
		rc.right - VOLUMEBAR_WIDTH, rc.bottom - VOLUMEBAR_HEIGHT,
		VOLUMEBAR_WIDTH, VOLUMEBAR_HEIGHT,
		hwndDlg,
		(HMENU)ID_VOLUMEBAR,
		g_hInst,
		NULL);

	SendMessage(g_hVolumebar, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(0, 100));

	SendMessage(g_hVolumebar, TBM_SETPAGESIZE, 0, (LPARAM)1);

	SendMessage(g_hVolumebar, TBM_SETPOS, (WPARAM)TRUE, g_pPlayer->GetVolumeLevel());


	g_hBackButton = CreateWindow(
		L"BUTTON", 
		L"Back",     
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_ICON,  
		rc.left,      
		rc.bottom-VOLUMEBAR_HEIGHT,    
		VOLUMEBAR_HEIGHT,  
		VOLUMEBAR_HEIGHT,
		hwndDlg,   
		(HMENU)ID_BACKBUTTON,    
		g_hInst,
		NULL);  
	
	HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON2));

	SendMessage(g_hBackButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);


	g_hPlayButton = CreateWindow(
		_T("BUTTON"),
		_T("Play"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_ICON,
		rc.left+VOLUMEBAR_HEIGHT,
		rc.bottom - VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		hwndDlg,
		(HMENU)ID_PLAYBUTTON,
		g_hInst,
		NULL);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON6));

	SendMessage(g_hPlayButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);


	g_hStopButton = CreateWindow(
		_T("BUTTON"),
		_T("Stop"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_ICON,
		rc.left + VOLUMEBAR_HEIGHT*2,
		rc.bottom - VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		hwndDlg,
		(HMENU)ID_STOPBUTTON,
		g_hInst,
		NULL);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON7));


	SendMessage(g_hStopButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);


	g_hForwardButton = CreateWindow(
		_T("BUTTON"),
		_T("Forward"),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_ICON,
		rc.left + VOLUMEBAR_HEIGHT * 3,
		rc.bottom - VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		hwndDlg,
		(HMENU)ID_FORWARDBUTTON,
		g_hInst,
		NULL);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON3));


	SendMessage(g_hForwardButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);


	TCHAR szNewTitle[64];
	size_t duration = g_pPlayer->GetDuration();
	size_t time = g_pPlayer->GetTime();
	StringCbPrintf(szNewTitle, sizeof(szNewTitle), _T("%d:%02d/%d:%02d"), time / 60, time % 60, duration / 60, duration % 60);

	g_hLabel = CreateWindow(
		_T("STATIC"),
		szNewTitle,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_CENTER | SS_NOTIFY,
		rc.left + VOLUMEBAR_HEIGHT*4,
		rc.bottom - VOLUMEBAR_HEIGHT,
		rc.right - VOLUMEBAR_WIDTH - VOLUMEBAR_HEIGHT*5,
		VOLUMEBAR_HEIGHT,
		hwndDlg,
		NULL,
		g_hInst,
		NULL);


	g_hVolume = CreateWindow(
		_T("STATIC"),
		szNewTitle,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | SS_ICON | SS_NOTIFY,
		rc.right - VOLUMEBAR_WIDTH - VOLUMEBAR_HEIGHT,
		rc.bottom - VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		VOLUMEBAR_HEIGHT,
		hwndDlg,
		NULL,
		g_hInst,
		NULL);

	hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON8));

	SendMessage(g_hVolume, STM_SETICON, (WPARAM)hIcon, 0);

}


void DestroyControlBar()
{
	if (g_hTrackbar != NULL)
	{
		EnableWindow(g_hTrackbar, FALSE);
		DestroyWindow(g_hTrackbar);
		g_hTrackbar = NULL;
	}

	if (g_hVolumebar != NULL)
	{
		EnableWindow(g_hVolumebar, FALSE);
		DestroyWindow(g_hVolumebar);
		g_hVolumebar = NULL;
	}

	if (g_hBackButton != NULL)
	{
		EnableWindow(g_hBackButton, FALSE);
		DestroyWindow(g_hBackButton);
		g_hBackButton = NULL;
	}

	if (g_hPlayButton != NULL)
	{
		EnableWindow(g_hPlayButton, FALSE);
		DestroyWindow(g_hPlayButton);
		g_hPlayButton = NULL;
	}

	if (g_hStopButton != NULL)
	{
		EnableWindow(g_hStopButton, FALSE);
		DestroyWindow(g_hStopButton);
		g_hStopButton = NULL;
	}

	if (g_hForwardButton != NULL)
	{
		EnableWindow(g_hForwardButton, FALSE);
		DestroyWindow(g_hForwardButton);
		g_hForwardButton = NULL;
	}

	if (g_hLabel != NULL)
	{
		EnableWindow(g_hLabel, FALSE);
		DestroyWindow(g_hLabel);
		g_hLabel = NULL;
	}

	if (g_hVolume != NULL)
	{
		EnableWindow(g_hVolume, FALSE);
		DestroyWindow(g_hVolume);
		g_hVolume = NULL;
	}
}


void PlayPause(HWND hwnd)
{
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
}


void Stop(HWND hwnd)
{
	KillTimer(hwnd, IDT_TIMER);
	g_pPlayer->Stop();
	SendMessage(g_hTrackbar, TBM_SETPOS, (WPARAM)TRUE, 0);
	SetWindowText(hwnd, szTitle);
}


void FileOpen(HWND hwnd, TCHAR* szFileName)
{
	HRESULT hr;
	hr = g_pPlayer->OpenFile(szFileName);
	if (FAILED(hr))
	{
		MessageBox(hwnd, _T("Cannot open file"), _T("Error"), MB_ICONERROR);
		SetFocus(hwnd);
		return;
	}
	InvalidateRect(hwnd, NULL, FALSE);
	OnSize(hwnd);
}


void CMDStart(HWND hwnd)
{
	int argc;
	TCHAR* CMDLine = GetCommandLine();
	TCHAR** argv = CommandLineToArgvW(CMDLine, &argc);
	if (argc > 1)
	{
		FileOpen(hwnd, argv[1]);
		g_pPlayer->Play();
		KillTimer(hwnd, IDT_TIMER);
		SetTimer(hwnd, IDT_TIMER, TICK_FREQ, (TIMERPROC)NULL);
	}
}