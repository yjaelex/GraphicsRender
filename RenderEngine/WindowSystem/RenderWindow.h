#ifndef _RENDER_WINDOW_H_
#define _RENDER_WINDOW_H_

#include <grtypes.h>
#include <string>
#include <map>
using namespace std;

class RenderWindow
{
public:
    RenderWindow();
    ~RenderWindow() { };

    // Creates & displays the new window.
    //    @param
    //        depthBuffer Specify true to include a depth-buffer.
    //    @param
    //        miscParam A variable number of pointers to platform-specific arguments. The
    //        actual requirements must be defined by the implementing subclasses.
    //
	virtual void create(const string & name, uint32 width, uint32 height, bool doubleBuffer, 
	        bool fullScreen, const NameValuePairList *miscParams) = 0;

	/** Alter fullscreen mode options. 
	@note Nothing will happen unless the settings here are different from the
		current settings.
	@param fullScreen Whether to use fullscreen mode or not. 
	@param width The new width to use
	@param height The new height to use
	*/
	virtual void setFullscreen(bool fullScreen, unsigned int width, unsigned int height) = 0;
        
    /** Destroys the window.
    */
    virtual void destroy(void) = 0;

    /** Alter the size of the window.
    */
    virtual void resize(unsigned int width, unsigned int height) = 0;

	/** Enable or disable vertical sync for the RenderWindow.
	*/
	virtual void setVSyncEnabled(bool vsync)
	{ m_VsyncEnable = vsync; }

	virtual bool isVSyncEnabled() const { return m_VsyncEnable; }

	/** Set the vertical sync interval. This indicates the number of vertical retraces to wait for
		before swapping buffers. A value of 1 is the default.
	*/
	virtual void setVSyncInterval(unsigned int interval)
	{ m_VSyncInterval = interval; }

	virtual unsigned int getVSyncInterval() const { return m_VSyncInterval; }

    virtual bool isFullScreen(void) const { return m_IsFullScreen; }

    virtual void getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth);

    virtual void windowMovedOrResized() { }

    virtual void swapBuffers() { }

    virtual void setHidden(bool hidden)
	{
		m_Hidden = hidden;
	}

    virtual void setActive( bool state )
    {
        m_Active = state;
    }

    void parseParam(const NameValuePairList * miscParams);

protected:
    bool    m_IsFullScreen;
    string  m_Title;
    uint32  m_Width;
    uint32  m_Height;
    int32   m_Top;
    int32   m_Left;
    bool    m_doubleBuffer;
    uint32  m_VSyncInterval;
    bool    m_VsyncEnable;
	bool    m_Active;
	bool    m_Closed;
    bool    m_Hidden;
};


#endif
