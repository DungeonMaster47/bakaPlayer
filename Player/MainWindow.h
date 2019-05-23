#pragma once
#include <windows.h>
#include <tchar.h>
#include "CommCtrl.h"

#define WINDOW_WIDTH 750
#define WINDOW_HEIGHT 500

#define TRACKBAR_SIZE 30

#define MENU_OPEN  0
#define MENU_ASPECT_RATIO 1
#define MENU_ABOUT 2

#define ID_TRACKBAR 10

#define IDT_TIMER 1
#define IDT_TIMER1 2
#define TICK_FREQ 250
#define TICK_FREQ1 1


static const TCHAR szWindowClass[] = _T("bakaPlayer");
static const TCHAR szTitle[] = _T("bakaPlayer");

VOID MenuInit(HMENU hMenu, HMENU hPopupMenu);
