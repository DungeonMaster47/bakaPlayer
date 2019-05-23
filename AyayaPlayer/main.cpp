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
	IGraphBuilder * pGraph(NULL);    //Интерфейс менеджера графа-фильтров
	IMediaControl * pControl(NULL);  //Интерфейс управления
	IMediaEvent   * pEvent(NULL);    //Интерфейс сообщений
	//Инициализируем библиотеку COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{
		//Создание менеджера графа-фильтров
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
		if (SUCCEEDED(hr))
		{
			//Получение интерфейса управления
			hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
			if (SUCCEEDED(hr))
			{
				//Получение интерфейса сообщений
				hr = pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
				if (SUCCEEDED(hr))
				{
					//Загрузка файла (Внимание! Unicode)
					WCHAR wFileName[MAX_PATH];
					MultiByteToWideChar(CP_ACP, 0, pcFileName, -1, wFileName, MAX_PATH);
					hr = pGraph->RenderFile((LPCWSTR)wFileName, NULL);

					//Для не Unicode
					//hr = pGraph->RenderFile((LPCWSTR)pcFileName, NULL);

					if (SUCCEEDED(hr))
					{
						//Запуск воспроизведения
						hr = pControl->Run();
						if (SUCCEEDED(hr))
						{
							//В реальном приложении INFINITE лучше не использовать
							long evCode;
							pEvent->WaitForCompletion(INFINITE, &evCode);
						}

						//Остановка графа-фильтров
						hr = pControl->Stop();
					}
				}
				else
				{
					printf("Ошибка: не удаётся получить интерфейс сообщений медиа!");
				}
			}
			else
			{
				printf("Ошибка: не удаётся получить интерфейс управления медиа!");
			}
		}
		else
		{
			printf("Ошибка: не удаётся создать менеджер графа-фильтров!");
		}

		//Освобождаем ресурсы
		if (pControl) pControl->Release();
		if (pEvent)   pEvent->Release();
		if (pGraph)   pGraph->Release();

		CoUninitialize();
	}
	else
	{
		printf("Ошибка: не удаётся инициализировать библиотеку COM!");
	}

	return hr;
}
//---------------------------------------------------------------------------