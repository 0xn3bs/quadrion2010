
#define NOMINMAX

#include <windows.h>

#include "qmath.h"
#include "qcamera.h"
#include "cINI.h"
#include "qmath.h"
#include "qrender.h"
#include "debug.h"
#include "app.h"
#include "playpen.h"


CApplication* g_pApp = NULL;



////////////////////////////////////////////////////////////////////////////////////
// WinMain 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Mem dump shit
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	
	// Initialization of MFC for nVidia PerfHUD (cheap workaround for perfHUD bug)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Load settings from config.ini //
	// Using a NULL callback (2nd param) uses a default system defined callback which only handles exits //
	g_pApp = new CApplication( "Media/config.ini", NULL );

	// preset game loop //
	MSG msg;
	ShowCursor(TRUE);

	//HACK: PlayInit() //
	PlayInit();

	// enter game loop
	bool done = false;
	while(!done)
	{
		// use PeekMessage so we dont get stuck waiting for one
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				done = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		else if(!done) 
		{
			// Do updates //
			
			g_pRender->BeginRendering();

			g_pRender->ClearBuffers(QRENDER_CLEAR_BACKBUFFER,  QRENDER_MAKE_ARGB(0, 0, 0, 0), 0);
			PlayRender();

			g_pRender->EndRendering();


			// Main loop logic goes here //
		}

		else
		{
			PostQuitMessage(0);
			done = true;
		}
	}
	
	PlayDestroy();
	QMem_SafeDelete( g_pApp );

	// Dump the leaks
	//_CrtDumpMemoryLeaks();
	//CloseHandle(hLogFile);
	
	//exit(0);
	return((int)msg.wParam);
}
