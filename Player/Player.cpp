#include "Player.h"



Player::Player(HWND hwnd) :
	m_state(STATE_NO_GRAPH),
	m_hwnd(hwnd),
	m_pGraph(NULL),
	m_pGraphBuilder(NULL),
	m_pControl(NULL),
	m_pEvent(NULL),
	m_pSeek(NULL),
	m_pWindow(NULL),
	m_pVideo(NULL),
	m_pAudio(NULL),
	m_pMediaType(NULL),
	m_bSaveAspectRatio(true)
{
}


Player::~Player()
{
	ReleaseGraph();
}


PlaybackState Player::State() const 
{
	return this->m_state;
}


HRESULT Player::Play()
{
	if(m_pGraph == nullptr)
	{
		return 0;
	}
	if (m_state == STATE_STOPPED)
	{
		LONGLONG current = 0;
		m_pSeek->SetPositions(&current, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
	}

	HRESULT hr = m_pControl->Run();
	if (SUCCEEDED(hr))
	{
		m_state = STATE_RUNNING;
	}
	return hr;
}


HRESULT Player::Pause()
{
	if (m_pGraph == nullptr)
	{
		return 0;
	}
	HRESULT hr = m_pControl->Pause();
	if (SUCCEEDED(hr))
	{
		m_state = STATE_PAUSED;
	}
	return hr;
}


HRESULT Player::Stop()
{
	if (m_pGraph == nullptr)
	{
		return 0;
	}
	HRESULT hr = m_pControl->Stop();
	if (SUCCEEDED(hr))
	{
		m_state = STATE_STOPPED;
	}
	return hr;
}


HRESULT Player::OpenFile(TCHAR* szFileName)
{
	ReleaseGraph();

	HRESULT hr;
	hr = InitializeGraph();
	if (FAILED(hr))
	{
		ReleaseGraph();
		return hr;
	}


	IBaseFilter *pSource;
	hr = m_pGraph->AddSourceFilter(szFileName, L"Source", &pSource);
	if (FAILED(hr))
	{
		return hr;
	}

	IBaseFilter *pVmr = NULL;
	hr = CoCreateInstance(CLSID_VideoMixingRenderer9, 0, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pVmr);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->AddFilter(pVmr, L"VMR9");
	if (FAILED(hr))
	{
		return hr;
	}


	//m_pGraph->RenderFile(szFileName, NULL);
	hr = m_pGraphBuilder->RenderStream(0, 0, pSource, 0, pVmr);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraphBuilder->RenderStream(0, &MEDIATYPE_Audio, pSource, 0, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pVideo));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pAudio));
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pWindow->put_Owner((OAHWND)m_hwnd);

	hr = m_pWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS);

	RECT rc;
	GetClientRect(m_hwnd, &rc);
	hr = m_pWindow->SetWindowPosition(0, 0, rc.right, (rc.bottom-TRACKBAR_SIZE));

	m_state = STATE_STOPPED;

	return 1;
}


HRESULT Player::InitializeGraph()
{
	ReleaseGraph();

	HRESULT hr;

	hr = InitCaptureGraphBuilder();
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pSeek));
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pControl));
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pEvent));
	if (FAILED(hr))
	{
		return hr;
	}
	hr = m_pGraph->QueryInterface(IID_PPV_ARGS(&m_pWindow));
	if (FAILED(hr))
	{
		return hr;
	}

	m_pGraph->SetDefaultSyncSource();

	hr = m_pEvent->SetNotifyWindow((OAHWND)m_hwnd, WM_GRAPH_EVENT, NULL);
	if (FAILED(hr))
	{
		return hr;
	}


	return 1;
}


void Player::ReleaseGraph()
{
	Stop();

	if (m_pMediaType)
	{
		m_pMediaType->Release();
		m_pMediaType = nullptr;
	}
	if (m_pEvent)
	{
		m_pEvent->SetNotifyWindow((OAHWND)NULL, NULL, NULL);
		m_pEvent->Release();
		m_pEvent = nullptr;
	}
	if (m_pWindow)
	{
		m_pWindow->put_Visible(OAFALSE);
		m_pWindow->put_Owner(NULL);
		m_pWindow->Release();
		m_pWindow = nullptr;
	}
	if (m_pGraph)
	{
		m_pGraph->Release();
		m_pGraph = nullptr;
	}
	if (m_pGraphBuilder)
	{
		m_pGraphBuilder->Release();
		m_pGraphBuilder = nullptr;
	}
	if (m_pControl)
	{
		m_pControl->Release();
		m_pControl = nullptr;
	}
	if (m_pSeek)
	{
		m_pSeek->Release();
		m_pSeek = nullptr;
	}
	if (m_pVideo)
	{
		m_pVideo->Release();
		m_pVideo = nullptr;
	}
	if (m_pAudio)
	{
		m_pAudio->Release();
		m_pAudio = nullptr;
	}

	m_state = STATE_NO_GRAPH;
}


HRESULT Player::HandleGraphEvent(GraphEventFN pfnOnGraphEvent)
{
	if (!m_pEvent)
	{
		return E_UNEXPECTED;
	}

	long evCode = 0;
	LONG_PTR param1 = 0, param2 = 0;

	HRESULT hr = S_OK;

	while (SUCCEEDED(m_pEvent->GetEvent(&evCode, &param1, &param2, 0)))
	{
		pfnOnGraphEvent(m_hwnd, evCode, param1, param2);

		hr = m_pEvent->FreeEventParams(evCode, param1, param2);
		if (FAILED(hr))
		{
			break;
		}
	}
	return hr;
}


HRESULT Player::HandleMoveEvent(OAHWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pGraph == nullptr)
	{
		return 0;
	}
	return m_pWindow->NotifyOwnerMessage(hWnd, uMsg, wParam, lParam);
}


HRESULT Player::UpdateVideoWindow(RECT rc)
{
	if (m_pGraph == nullptr || m_pVideo == nullptr || m_pWindow == nullptr)
	{
		return 0;
	}
	if (m_bSaveAspectRatio)
	{
		long width;
		long height;
		m_pVideo->GetVideoSize(&width, &height);
		if ((double)rc.right / (double)width < (double)(rc.bottom-TRACKBAR_SIZE) / (double)height)
		{
			m_pWindow->SetWindowPosition(0, (rc.bottom-TRACKBAR_SIZE)/2.0 - ((double)height *((double)rc.right / (double)width))/2.0, (double)width * ((double)rc.right / (double)width), (double)height *((double)rc.right / (double)width));
		}
		else
		{
			m_pWindow->SetWindowPosition(rc.right/2.0-(((double)width * ((double)(rc.bottom-TRACKBAR_SIZE) / height)) / 2.0), 0, (double)width * ((double)(rc.bottom-TRACKBAR_SIZE) / (double)height), (double)height * ((double)(rc.bottom-TRACKBAR_SIZE) / (double)height));
		}
	}
	else
		m_pWindow->SetWindowPosition(0, 0, rc.right, (rc.bottom-TRACKBAR_SIZE));

	return 1;
}


void Player::ChangeSaveAspectRatio()
{
	m_bSaveAspectRatio = !m_bSaveAspectRatio;
	if (m_state != STATE_NO_GRAPH)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		UpdateVideoWindow(rc);
	}
}


LONGLONG Player::GetDuration()
{
	if (m_state != STATE_NO_GRAPH)
	{
		LONGLONG duration;
		m_pSeek->GetDuration(&duration);
		return duration / ONE_SECOND;
	} 

	return 0;
}


LONGLONG Player::GetTime()
{
	if (m_state != STATE_NO_GRAPH)
	{
		LONGLONG current;
		m_pSeek->GetCurrentPosition(&current);
		return current / ONE_SECOND;
	}
	return 0;
}


void Player::SetPos(LONGLONG pos)
{
	if (m_state == STATE_NO_GRAPH)
		return;
	pos *= ONE_SECOND;
	m_pSeek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
}

HRESULT Player::InitCaptureGraphBuilder()
{
	m_pGraph = NULL;
	m_pGraphBuilder = NULL;

	HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL,
		CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void**)&m_pGraphBuilder);
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER,
			IID_IGraphBuilder, (void**)&m_pGraph);
		if (SUCCEEDED(hr))
		{
			m_pGraphBuilder->SetFiltergraph(m_pGraph);
			return S_OK;
		}
		else
		{
			m_pGraphBuilder->Release();
		}
	}
	return hr;
}