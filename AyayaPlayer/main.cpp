#pragma comment(lib, "Quartz.lib")
#pragma comment(lib, "strmiids.lib")
//---------------------------------------------------------------------------
#include <stdio.h>
#include <dshow.h>
//---------------------------------------------------------------------------
char pcFileName[] = "F:\\down\\GraphEdit\\cirno.avi";
//---------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	IGraphBuilder * pGraph(NULL);    //��������� ��������� �����-��������
	IMediaControl * pControl(NULL);  //��������� ����������
	IMediaEvent   * pEvent(NULL);    //��������� ���������
	//�������������� ���������� COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		//�������� ��������� �����-��������
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
		if (SUCCEEDED(hr))
		{
			//��������� ���������� ����������
			hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
			if (SUCCEEDED(hr))
			{
				//��������� ���������� ���������
				hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
				if (SUCCEEDED(hr))
				{
					//�������� ����� (��������! Unicode)
					WCHAR wFileName[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, pcFileName, -1, wFileName, MAX_PATH);
					hr = pGraph->RenderFile((LPCWSTR)wFileName, NULL);

					//��� �� Unicode
					//hr = pGraph->RenderFile((LPCWSTR)pcFileName, NULL);

					if (SUCCEEDED(hr))
					{
						//������ ���������������
						hr = pControl->Run();
						if (SUCCEEDED(hr))
						{
							//� �������� ���������� INFINITE ����� �� ������������
							long evCode;
							pEvent->WaitForCompletion(INFINITE, &evCode);
						}

						//��������� �����-��������
						hr = pControl->Stop();
					}
				}
				else
				{
					printf("������: �� ������ �������� ��������� ��������� �����!");
				}
			}
			else
			{
				printf("������: �� ������ �������� ��������� ���������� �����!");
			}
		}
		else
		{
			printf("������: �� ������ ������� �������� �����-��������!");
		}

		//����������� �������
		if (pControl) pControl->Release();
		if (pEvent)   pEvent->Release();
		if (pGraph)   pGraph->Release();

		CoUninitialize();
	}
	else
	{
		printf("������: �� ������ ���������������� ���������� COM!");
	}

	return hr;
}
//---------------------------------------------------------------------------