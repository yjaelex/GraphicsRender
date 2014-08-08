#include "WindowEvent.h"

WindowEventHandler::WindowEventListeners WindowEventHandler::m_sListeners;
WindowEventHandler::Windows WindowEventHandler::m_sWindows;

void WindowEventHandler::messagePump()
{
#ifdef _WIN32
	// Windows Message Loop (NULL means check all HWNDs belonging to this context)
	MSG  msg;
	while( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
#else 
#ifdef _LINUX
	//GLX Message Pump
	Windows::iterator win = m_sWindows.begin();
	Windows::iterator end = m_sWindows.end();

	Display* xDisplay = 0; // same for all windows
	
	for (; win != end; win++)
	{
	    XID xid;
	    XEvent event;

	    if (!xDisplay)
        {
		    (*win)->getCustomAttribute("XDISPLAY", &xDisplay);
        }

	    (*win)->getCustomAttribute("WINDOW", &xid);

	    while (XCheckWindowEvent (xDisplay, xid, StructureNotifyMask | VisibilityChangeMask | FocusChangeMask, &event))
	    {
            GLXProc(*win, event);
	    }

	    // The ClientMessage event does not appear under any Event Mask
	    while (XCheckTypedWindowEvent (xDisplay, xid, ClientMessage, &event))
	    {
		    GLXProc(*win, event);
	    }
	}
#endif
#endif

}

//--------------------------------------------------------------------------------//
void WindowEventHandler::addWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	m_sListeners.insert(std::make_pair(window, listener));
}

//--------------------------------------------------------------------------------//
void WindowEventHandler::removeWindowEventListener( RenderWindow* window, WindowEventListener* listener )
{
	WindowEventListeners::iterator i = m_sListeners.begin(), e = m_sListeners.end();

	for( ; i != e; ++i )
	{
		if( i->first == window && i->second == listener )
		{
			m_sListeners.erase(i);
			break;
		}
	}
}

//--------------------------------------------------------------------------------//
void WindowEventHandler::addRenderWindow(RenderWindow* window)
{
	m_sWindows.push_back(window);
}

//--------------------------------------------------------------------------------//
void WindowEventHandler::removeRenderWindow(RenderWindow* window)
{
	Windows::iterator i = std::find(m_sWindows.begin(), m_sWindows.end(), window);
	if( i != m_sWindows.end() )
    {
		m_sWindows.erase( i );
    }
}

#ifdef _WIN32
LRESULT CALLBACK WindowEventHandler::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{	// Store pointer to Win32Window in user data area
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams));
		return 0;
	}

	// look up window instance
	// note: it is possible to get a WM_SIZE before WM_CREATE
	RenderWindow* win = (RenderWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!win)
    {
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

	//Iterator of all listeners registered to this RenderWindow
	WindowEventListeners::iterator index,
    start = m_sListeners.lower_bound(win),
    end = m_sListeners.upper_bound(win);

	switch( uMsg )
	{
	case WM_ACTIVATE:
	{
        bool active = (LOWORD(wParam) != WA_INACTIVE);
        if( active )
        {
        win->setActive( true );
        }
        else
        {
            win->setActive( false );
        }

	    for( ; start != end; ++start )
		    (start->second)->windowFocusChange(win);
		break;
	}
	case WM_SYSKEYDOWN:
		switch( wParam )
		{
		case VK_CONTROL:
		case VK_SHIFT:
		case VK_MENU: //ALT
			//return zero to bypass defProc and signal we processed the message
			return 0;
		}
		break;
	case WM_SYSKEYUP:
		switch( wParam )
		{
		case VK_CONTROL:
		case VK_SHIFT:
		case VK_MENU: //ALT
		case VK_F10:
			//return zero to bypass defProc and signal we processed the message
			return 0;
		}
		break;
	case WM_SYSCHAR:
		// return zero to bypass defProc and signal we processed the message, unless it's an ALT-space
		if (wParam != VK_SPACE)
			return 0;
		break;
	case WM_ENTERSIZEMOVE:
		//log->logMessage("WM_ENTERSIZEMOVE");
		break;
	case WM_EXITSIZEMOVE:
		//log->logMessage("WM_EXITSIZEMOVE");
		break;
	case WM_MOVE:
		//log->logMessage("WM_MOVE");
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
        {
			(index->second)->windowMoved(win);
        }
		break;
	case WM_DISPLAYCHANGE:
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
        {
			(index->second)->windowResized(win);
        }
		break;
	case WM_SIZE:
		//log->logMessage("WM_SIZE");
		win->windowMovedOrResized();
		for(index = start; index != end; ++index)
        {
			(index->second)->windowResized(win);
        }
		break;
	case WM_GETMINMAXINFO:
		// Prevent the window from going smaller than some minimu size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 100;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 100;
		break;
	case WM_CLOSE:
	{
		//log->logMessage("WM_CLOSE");
		bool close = true;
		for(index = start; index != end; ++index)
		{
			if (!(index->second)->windowClosing(win))
            {
				close = false;
            }
		}
		if (!close) return 0;

		for(index = m_sListeners.lower_bound(win); index != end; ++index)
        {
			(index->second)->windowClosed(win);
        }
		win->destroy();
		return 0;
	}
	}

	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

#else 
#ifdef _LINUX
void GLXProc( RenderWindow *win, const XEvent &event )
{
	//An iterator for the window listeners
	WindowEventListeners::iterator index,
    start = m_sListeners.lower_bound(win),
    end = m_sListeners.upper_bound(win);

    switch(event.type)
	{
	case ClientMessage:
	{
		::Atom atom;
		win->getCustomAttribute("ATOM", &atom);
		if(event.xclient.format == 32 && event.xclient.data.l[0] == (long)atom)
		{	//Window closed by window manager
			//Send message first, to allow app chance to unregister things that need done before
			//window is shutdown
			bool close = true;
            for(index = start ; index != end; ++index)
			{
				if (!(index->second)->windowClosing(win))
					close = false;
			}
			if (!close) return;

            for(index = start ; index != end; ++index)
                (index->second)->windowClosed(win);
			win->destroy();
		}
		break;
	}
	case DestroyNotify:
	{
		if (!win->isClosed())
		{
			// Window closed without window manager warning.
            for(index = start ; index != end; ++index)
                (index->second)->windowClosed(win);
			win->destroy();
		}
		break;
	}
	case ConfigureNotify:
	{    
        // This could be slightly more efficient if windowMovedOrResized took arguments:
		unsigned int oldWidth, oldHeight, oldDepth;
		int oldLeft, oldTop;
		win->getMetrics(oldWidth, oldHeight, oldDepth, oldLeft, oldTop);
		win->windowMovedOrResized();

		unsigned int newWidth, newHeight, newDepth;
		int newLeft, newTop;
		win->getMetrics(newWidth, newHeight, newDepth, newLeft, newTop);

		if (newLeft != oldLeft || newTop != oldTop)
		{
            for(index = start ; index != end; ++index)
                (index->second)->windowMoved(win);
		}

		if (newWidth != oldWidth || newHeight != oldHeight)
		{
            for(index = start ; index != end; ++index)
                (index->second)->windowResized(win);
		}
		break;
	}
	case FocusIn:     // Gained keyboard focus
	case FocusOut:    // Lost keyboard focus
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case MapNotify:   //Restored
		win->setActive( true );
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case UnmapNotify: //Minimised
		win->setActive( false );
		win->setVisible( false );
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	case VisibilityNotify:
		switch(event.xvisibility.state)
		{
		case VisibilityUnobscured:
			win->setActive( true );
			win->setVisible( true );
			break;
		case VisibilityPartiallyObscured:
			win->setActive( true );
			win->setVisible( true );
			break;
		case VisibilityFullyObscured:
			win->setActive( false );
			win->setVisible( false );
			break;
		}
        for(index = start ; index != end; ++index)
            (index->second)->windowFocusChange(win);
		break;
	default:
		break;
	} //End switch event.type
}
#endif
#endif
