#ifndef _WINDOW_EVENT_H_
#define _WINDOW_EVENT_H_


#include <grtypes.h>
#include "RenderWindow.h"

#ifdef _WIN32
#include "windows.h"
#endif

#ifdef LINUX
#include <X11/Xlib.h>
void GLXProc( RenderWindow *win, const XEvent &event );
#endif

//Callback class used to send out window events to client app
class WindowEventListener
{
public:
	virtual ~WindowEventListener() {}

	virtual void windowMoved(RenderWindow* rw)
    { (void)rw; }

	virtual void windowResized(RenderWindow* rw)
    { (void)rw; }

    // Window is closing (Only triggered if user pressed the [X] button)
	virtual bool windowClosing(RenderWindow* rw)
	{ (void)rw; return true; }

	virtual void windowClosed(RenderWindow* rw)
    { (void)rw; }

    // Window has lost/gained focus
	virtual void windowFocusChange(RenderWindow* rw)
    { (void)rw; }
};


// Utility class to handle Window Events/Pumping/Messages
class WindowEventHandler
{
public:
	static void messagePump();

	//Add a listener to listen to renderwindow events (multiple listener's per renderwindow is fine)
	//The same listener can listen to multiple windows, as the Window Pointer is sent along with
	//any messages.
	//@param window
	//	The RenderWindow you are interested in monitoring
	//@param listner
	//	Your callback listener
	static void addWindowEventListener( RenderWindow* window, WindowEventListener* listener );

	static void removeWindowEventListener( RenderWindow* window, WindowEventListener* listener );

	// Called by RenderWindows upon creation.
	static void addRenderWindow(RenderWindow* window);

	static void removeRenderWindow(RenderWindow* window);

#ifdef _WIN32
	//! Internal winProc (RenderWindow's use this when creating the Win32 Window)
	static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

	//These are public only so GLXProc can access them without adding Xlib headers header
	typedef multimap<RenderWindow*, WindowEventListener*> WindowEventListeners;
	static WindowEventListeners m_sListeners;

	typedef vector<RenderWindow*> Windows;
	static Windows m_sWindows;
};

#endif
