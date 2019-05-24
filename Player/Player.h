#pragma once
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include "MainWindow.h"
#pragma comment(lib, "Quartz.lib")
#pragma comment(lib, "strmiids.lib")
#define WM_GRAPH_EVENT WM_APP+1

#define ONE_SECOND 10000000

enum PlaybackState
{
	STATE_NO_GRAPH,
	STATE_RUNNING,
	STATE_PAUSED,
	STATE_STOPPED,
};


typedef void (CALLBACK *GraphEventFN)(HWND hwnd, long eventCode, LONG_PTR param1, LONG_PTR param2);

class Player
{
private:
	PlaybackState  m_state;
	HWND		   m_hwnd;
	IGraphBuilder* m_pGraph;
	ICaptureGraphBuilder2* m_pGraphBuilder;
	IMediaControl* m_pControl;
	IMediaEventEx* m_pEvent;
	IMediaSeeking* m_pSeek;
	IVideoWindow*  m_pWindow;
	IBasicVideo*   m_pVideo;
	IBasicAudio*   m_pAudio;
	void		   ReleaseGraph();
	HRESULT		   InitializeGraph();
	bool		   m_bSaveAspectRatio;
	HRESULT InitCaptureGraphBuilder();
public:
	Player(HWND hWnd);
	~Player();
	PlaybackState State() const;
	HRESULT Play();
	HRESULT Stop();
	HRESULT Pause();
	HRESULT OpenFile(TCHAR* szFileName);
	HRESULT HandleGraphEvent(GraphEventFN pfnOnGraphEvent);
	HRESULT HandleMoveEvent(OAHWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT UpdateVideoWindow(RECT rc);
	void	ChangeSaveAspectRatio();
	LONGLONG	GetDuration();
	LONGLONG    GetTime();
	void		SetPos(LONGLONG pos);
};

