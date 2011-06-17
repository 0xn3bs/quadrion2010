#include "app.h"


static LRESULT CALLBACK DefaultEventCallback(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		switch(uMsg)
	{
		case WM_QUIT:
		{
			PostQuitMessage(0);
//			QMem_SafeDelete( g_pApp );
//			killWindow();
		}
	
		case WM_SYSCOMMAND:
		{
			if(wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER)
				return 0;
			
			break;
		}
		
		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}


		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
			break;
		}	
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


CApplicationWindow::CApplicationWindow()
{
	LoadFromDefaults();
}

CApplicationWindow::CApplicationWindow( cINI* ini, WNDPROC wndProc )
{
	LoadFromINI( ini, wndProc );
}



CApplicationWindow::~CApplicationWindow()
{
	DestroyWindow( handle );
	instance = NULL;
}


void CApplicationWindow::LoadFromINI( cINI* ini, WNDPROC wndProc )
{
	// Populate window class
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT windowRect;
	windowRect.left = (long)0;
	windowRect.right = (long)ini->queryInt("resolution_x");
	windowRect.top = (long)0;
	windowRect.bottom = (long)ini->queryInt("resolution_y");
	
	WNDPROC cbFunc = ( wndProc ) ? wndProc : (WNDPROC)DefaultEventCallback;

	instance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = cbFunc;//(WNDPROC)DefaultEventCallback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "SMSENGINE";
	
	// Register window class
	if(!RegisterClass(&wc))
		returnError("Failed to register window class", "Quit Error");
	
	// set DEVMODE settings for fullscreen
	if(ini->queryBool("fullscreen"))
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = ini->queryInt("resolution_x");
		dmScreenSettings.dmPelsHeight = ini->queryInt("resolution_y");
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		
		if(ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			bool INI_FULLSCREEN = ini->queryBool("fullscreen");
			systemToUser("Fullscreen not available, use windowed mode?", "?", INI_FULLSCREEN);
			if(ini->queryBool("fullscreen"))
				returnError("Program will exit", "Quit Error");
			INI_FULLSCREEN = !INI_FULLSCREEN;

			if(INI_FULLSCREEN)
				ini->setValue("fullscreen", "true");
			else
				ini->setValue("fullscreen", "false");
		}	
	}
	
	if(ini->queryBool("fullscreen"))
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
	}
	
	// size window and create a window object
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
	
	LPCSTR name = ini->queryString( "name" ).c_str();
	if(!(handle = CreateWindowEx(dwExStyle, "SMSENGINE", name, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
							     0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
							     NULL, NULL, instance, NULL)))
	{
		returnError("System could not create window", "Quit Error");
	}

	SQuadrionRenderInitializer init;
	init.displayWidth = (UINT)ini->queryInt("resolution_x");
	init.displayHeight = (UINT)ini->queryInt("resolution_y");
	init.bufferFormat = QRENDER_BUFFER_FORMAT_X8R8G8B8;
	init.bufferCount = 1;
	init.multisample = QRENDER_MULTISAMPLE_NONE;
	init.backbufferFormat = QRENDER_DEPTH_24_STENCIL_8;
	init.windowed = !ini->queryBool("fullscreen");
	init.vsync = ini->queryBool("vsync");
	init.debugGraphics = ini->queryBool("debuggraphics");
	init.fsaa = ini->queryInt("fsaa");
	init.csaa = ini->queryInt( "csaa" );
	g_pRender->Initialize(handle, init);

	ShowWindow(handle, SW_SHOW);
	SetForegroundWindow(handle);
	SetFocus(handle);	
}


void CApplicationWindow::LoadFromDefaults()
{
	// Populate window class
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT windowRect;
	windowRect.left = (long)0;
	windowRect.right = 512;
	windowRect.top = (long)0;
	windowRect.bottom = 512;
	
	instance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)DefaultEventCallback;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = instance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "SMSENGINE";
	
	// Register window class
	if(!RegisterClass(&wc))
		returnError("Failed to register window class", "Quit Error");

	dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle = WS_OVERLAPPEDWINDOW;
	
	
	// size window and create a window object
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);
	
	LPCSTR name = "Default";
	if(!(handle = CreateWindowEx(dwExStyle, "SMSENGINE", name, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
							     0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
							     NULL, NULL, instance, NULL)))
	{
		returnError("System could not create window", "Quit Error");
	}

	SQuadrionRenderInitializer init;
	init.displayWidth = 512;
	init.displayHeight = 512;
	init.bufferFormat = QRENDER_BUFFER_FORMAT_X8R8G8B8;
	init.bufferCount = 1;
	init.multisample = QRENDER_MULTISAMPLE_NONE;
	init.backbufferFormat = QRENDER_DEPTH_24_STENCIL_8;
	init.windowed = true;
	init.vsync = false;
	init.debugGraphics = false;
	init.fsaa = 0;
	init.csaa = 0;
	g_pRender->Initialize(handle, init);

	ShowWindow(handle, SW_SHOW);
	SetForegroundWindow(handle);
	SetFocus(handle);	
}



CApplication::CApplication()
{
	appINI = NULL;
	LoadDefaults();
}

CApplication::CApplication( const char* fName, WNDPROC wndProc )
{
	appINI = new cINI();
	if( fName )
	{
		if( appINI->load( std::string( fName ) ) )
		{
			appWindow = new CApplicationWindow( appINI, wndProc );
		}

		else
			LoadDefaults();
	}

	else
		LoadDefaults();
}


CApplication::~CApplication()
{
	QMem_SafeDelete( appINI );
	QMem_SafeDelete( appWindow );
}


void CApplication::LoadDefaults()
{
	appWindow = new CApplicationWindow();
}


const cINI* CApplication::GetApplicationINI()
{
	return appINI;
}


int CApplication::GetWindowWidth()
{
	return appWindow->GetWidth();
}

int CApplication::GetWindowHeight()
{
	return appWindow->GetHeight();
}

void CApplication::GetMousePosition( int& x, int& y )
{
	POINT pt;
	if( GetCursorPos( &pt ) )
	{
		x = pt.x;
		y = pt.y;
		return;
	}

	else 
		return;
}

void CApplication::SetMousePosition( int x, int y )
{
	SetCursorPos( x, y );
}