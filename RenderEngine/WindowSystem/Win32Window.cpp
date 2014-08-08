
#include "Win32Window.h"


void Win32Window::create(const string& name, unsigned int width, unsigned int height, bool doubleBuffer,
						bool fullScreen, const NameValuePairList *miscParams)
{
	// destroy current window, if any
	if (m_HWnd)
    {
		destroy();
    }

	m_Title = name;
	m_IsFullScreen = fullScreen;
	DWORD		  dwStyleEx = 0;					
	MONITORINFOEX monitorInfoEx;
	RECT		  rc;
    HINSTANCE hInst = GetModuleHandle( NULL );

	int32 left = -1;                    // Defaults to screen center
	int32 top = -1;                     // Defaults to screen center
    bool hidden = false;
    HWND parent = 0;
    uint32  border = 0;
	
    // Select monitor first.
	// If we didn't specified the adapter index
	if (m_HMonitor == NULL)
	{
		POINT windowAnchorPoint;

		// Fill in anchor point.
		windowAnchorPoint.x = left;
		windowAnchorPoint.y = top;
		m_HMonitor = MonitorFromPoint(windowAnchorPoint, MONITOR_DEFAULTTOPRIMARY);
	}

	// Get the target monitor info		
	memset(&monitorInfoEx, 0, sizeof(MONITORINFOEX));
	monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(m_HMonitor, &monitorInfoEx);
    m_DeviceName.assign(monitorInfoEx.szDevice);

	// Update window style flags.
	m_FullscreenWinStyle = (hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN | WS_POPUP;
	m_WindowedWinStyle   = (hidden ? 0 : WS_VISIBLE) | WS_CLIPCHILDREN;
			
	if (parent)
	{
		m_WindowedWinStyle |= WS_CHILD;
	}
	else
	{
        // None border
		if (border == 0)
			m_WindowedWinStyle |= WS_POPUP;
        // Fixed border
		else if (border == 1)
			m_WindowedWinStyle |= WS_OVERLAPPED | WS_BORDER | WS_CAPTION |
			WS_SYSMENU | WS_MINIMIZEBOX;
		else
			m_WindowedWinStyle |= WS_OVERLAPPEDWINDOW;
	}

	// No specified top left -> Center the window in the middle of the monitor
	if (left == -1 || top == -1)
	{				
		int screenw = monitorInfoEx.rcWork.right  - monitorInfoEx.rcWork.left;
		int screenh = monitorInfoEx.rcWork.bottom - monitorInfoEx.rcWork.top;

		if (left == -1)
        {
		    left = monitorInfoEx.rcWork.left + (screenw - width) / 2;
        }
		if (top == -1)
        {
		    top = monitorInfoEx.rcWork.top + (screenh - height) / 2;
        }
	}

	m_Width = width;
	m_Height = height;
	m_Top = top;
	m_Left = left;

	if (m_IsFullScreen)
	{				
		dwStyleEx |= WS_EX_TOPMOST;
		m_Top = monitorInfoEx.rcMonitor.top;
		m_Left = monitorInfoEx.rcMonitor.left;											
	}
    else
    {
        RECT rc;
	    SetRect(&rc, 0, 0, m_Width, m_Height);
	    AdjustWindowRect(&rc, getWindowStyle(fullScreen), false);
	    m_Width = rc.right - rc.left;
	    m_Height = rc.bottom - rc.top;
    }
 
	UINT classStyle = CS_OWNDC | CS_DBLCLKS;

	// register class and create window
	WNDCLASS wc = { classStyle, WindowEventHandler::_WndProc, 0, 0, hInst,
		LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(BLACK_BRUSH), NULL, "GaphicsRenderWindow" };
	RegisterClass(&wc);

	if (m_IsFullScreen)
	{
		DEVMODE displayDeviceMode;

		memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
		displayDeviceMode.dmSize = sizeof(DEVMODE);
		displayDeviceMode.dmBitsPerPel = 32;
		displayDeviceMode.dmPelsWidth = m_Width;
		displayDeviceMode.dmPelsHeight = m_Height;
		displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (ChangeDisplaySettingsEx(m_DeviceName.c_str(), &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL) != DISP_CHANGE_SUCCESSFUL)
        {
			// logMessage(LML_CRITICAL, "ChangeDisplaySettings failed");
        }
	}

	// Pass pointer to self as WM_CREATE parameter
	m_HWnd = CreateWindowEx(dwStyleEx, "GaphicsRenderWindow", m_Title.c_str(),
		getWindowStyle(fullScreen), m_Left, m_Top, m_Width, m_Height, parent, 0, hInst, this);

	WindowEventHandler::addRenderWindow(this);

    // top and left represent outer window position
    GetWindowRect(m_HWnd, &rc);
    m_Top = rc.top;
    m_Left = rc.left;
    // width and height represent drawable area only
    GetClientRect(m_HWnd, &rc);
    m_Width = rc.right;
    m_Height = rc.bottom;
    m_HDC = GetDC(m_HWnd);

    // Create RenderSystem context
    // mContext = new Win32Context(mHDC, mGlrc);

    m_Active = true;
    setHidden(false);
}


void Win32Window::destroy(void)
{
	if (!m_HWnd)     return;

	WindowEventHandler::removeRenderWindow(this);

	if (m_IsFullScreen)
    {
        ChangeDisplaySettingsEx(m_DeviceName.c_str(), NULL, NULL, 0, NULL);
    }
	DestroyWindow(m_HWnd);

	m_Active = false;
	m_Closed = true;
	m_HWnd = 0;
    m_HMonitor = NULL;
    m_DeviceName.clear();
}

void Win32Window::resize(unsigned int width, unsigned int height)
{
	if (m_HWnd && !m_IsFullScreen)
	{
		RECT rc = { 0, 0, width, height };
		AdjustWindowRect(&rc, getWindowStyle(m_IsFullScreen), false);
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		SetWindowPos(m_HWnd, 0, 0, 0, width, height,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void Win32Window::windowMovedOrResized()
{
	if (!m_HWnd || IsIconic(m_HWnd))
		return;

	updateWindowRect();		
}


void Win32Window::updateWindowRect()
{
	RECT rc;
	BOOL result;

	// Update top left parameters
	result = GetWindowRect(m_HWnd, &rc);
	if (result == FALSE)
	{
		m_Top = 0;
		m_Left = 0;
		m_Width = 0;
		m_Height = 0;
		return;
	}

	m_Top = rc.top;
	m_Left = rc.left;

	// width and height represent drawable area only
	result = GetClientRect(m_HWnd, &rc);
	if (result == FALSE)
	{
		m_Top = 0;
		m_Left = 0;
		m_Width = 0;
		m_Height = 0;
		return;
	}
	unsigned int width = rc.right - rc.left;
	unsigned int height = rc.bottom - rc.top;

	// Case window resized.
	if (width != m_Width || height != m_Height)
	{
		m_Width  = rc.right - rc.left;
		m_Height = rc.bottom - rc.top;

		// Notify viewports of resize
/*		ViewportList::iterator it = mViewportList.begin();
		while( it != mViewportList.end() )
			(*it++).second->_updateDimensions();*/			
	}
}

void Win32Window::setActive( bool state )
{	
	m_Active = state;

	if( m_IsFullScreen )
	{
		if( state == false )
		{	//Restore Desktop
            ChangeDisplaySettingsEx(m_DeviceName.c_str(), NULL, NULL, 0, NULL);
			ShowWindow(m_HWnd, SW_SHOWMINNOACTIVE);
		}
		else
		{	//Restore App
			ShowWindow(m_HWnd, SW_SHOWNORMAL);

			DEVMODE displayDeviceMode;

			memset(&displayDeviceMode, 0, sizeof(displayDeviceMode));
			displayDeviceMode.dmSize = sizeof(DEVMODE);
			displayDeviceMode.dmBitsPerPel = 32;
			displayDeviceMode.dmPelsWidth = m_Width;
			displayDeviceMode.dmPelsHeight = m_Height;
			displayDeviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
            ChangeDisplaySettingsEx(m_DeviceName.c_str(), &displayDeviceMode, NULL, CDS_FULLSCREEN, NULL);
		}
	}
}

void Win32Window::swapBuffers()
{
	SwapBuffers(m_HDC);
}