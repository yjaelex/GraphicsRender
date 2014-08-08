#ifndef _WIN32_RENDER_WINDOW_H_
#define _WIN32_RENDER_WINDOW_H_


#include "RenderWindow.h"
#include "windows.h"
#include "WindowEvent.h"

class Win32Window : public Window
{
public:
    Win32Window() { m_HWnd = NULL; m_HInst = NULL; m_HMonitor = NULL; m_HDC = NULL; }

	virtual void create(const string & name, uint32 width, uint32 height, bool doubleBuffer, 
	        bool fullScreen, const NameValuePairList *miscParams);

	virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height);

    virtual void destroy(void);

    virtual void resize(unsigned int width, unsigned int height);

    virtual void windowMovedOrResized();

    virtual void setHidden(bool hidden)
	{
		m_Hidden = hidden;
        ShowWindow(m_HWnd, hidden ? SW_HIDE: SW_SHOWNORMAL);
	}

    virtual void setActive( bool state );
    virtual void swapBuffers();

protected:
    uint32 getWindowStyle(bool fullScreen) const { if (fullScreen) return m_FullscreenWinStyle; return m_WindowedWinStyle; }
    void updateWindowRect();

protected:
    HWND        m_HWnd;
    HINSTANCE   m_HInst;
    HMONITOR    m_HMonitor;
    string      m_DeviceName;
    uint32      m_FullscreenWinStyle;
    uint32      m_WindowedWinStyle;
    HDC         m_HDC;
}

#endif
