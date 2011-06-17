#ifndef __APP_H_
#define __APP_H_

#include <Windows.h>
#include "cINI.h"
#include "qmem.h"
#include "debug.h"



class CApplicationWindow
{
	public:

		CApplicationWindow();
		CApplicationWindow( cINI* ini, WNDPROC wndProc = NULL );
		~CApplicationWindow();

		int GetWidth() { return winWidth; }
		int GetHeight() { return winHeight; }

	protected:

		int			winWidth;
		int			winHeight;

	private:

		void		LoadFromINI( cINI* ini, WNDPROC wndProc = NULL );
		void		LoadFromDefaults();


		HWND		handle;
		HINSTANCE	instance;

		WNDPROC		eventCallback;
};



class CApplication
{
	public:

		/**
		  *  Default constructor loads application and window from default (safe) settings
		**/
		CApplication();

		/**
		  *	Constructor with INI file name. Loads initial settings from valid INI file 
		  * If wndProc is NULL, the system will use a default callback which only handles exits
		**/
		CApplication( const char* fName, WNDPROC wndProc = NULL );		

			
		~CApplication();


		/**
		  * Obtain the initialized INI file 
		**/
		const cINI* GetApplicationINI();


		int		GetWindowWidth();
		int		GetWindowHeight();

		void	GetMousePosition( int& x, int& y );
		void	SetMousePosition( int x, int y);

	protected:

	private:

		void					LoadDefaults();

		CApplicationWindow*		appWindow;
		cINI*					appINI;
};


extern CApplication* g_pApp;


#endif