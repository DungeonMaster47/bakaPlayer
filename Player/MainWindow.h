#pragma once
#include <windows.h>
#include <tchar.h>
#include "CommCtrl.h"

#define WINDOW_WIDTH 750
#define WINDOW_HEIGHT 500

#define TRACKBAR_HEIGHT 60
#define VOLUMEBAR_HEIGHT 30
#define VOLUMEBAR_WIDTH  50

#define MENU_OPEN  1
#define MENU_ASPECT_RATIO 2
#define MENU_ABOUT 3

#define ID_TRACKBAR 10
#define ID_VOLUMEBAR 11
#define ID_BACKBUTTON 12
#define ID_PLAYBUTTON 13
#define ID_STOPBUTTON 14
#define ID_FORWARDBUTTON 15

#define IDT_TIMER 1
#define IDT_TIMER1 2
#define TICK_FREQ 250
#define TICK_FREQ1 1


static const TCHAR szWindowClass[] = _T("bakaPlayer");
static const TCHAR szTitle[] = _T("bakaPlayer");

VOID MenuInit(HMENU hMenu, HMENU hPopupMenu);
