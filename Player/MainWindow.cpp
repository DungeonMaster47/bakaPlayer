#include "MainWindow.h"

void MenuInit(HMENU hMenu, HMENU hPopupMenu)
{
	AppendMenu(hMenu, MF_STRING, MENU_OPEN, _T("&Open"));
	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hPopupMenu, _T("&Settings"));
	AppendMenu(hPopupMenu, MF_STRING | MF_CHECKED, MENU_ASPECT_RATIO, _T("Keep aspect ratio"));
	AppendMenu(hMenu, MF_STRING, MENU_ABOUT, _T("&About"));
}


