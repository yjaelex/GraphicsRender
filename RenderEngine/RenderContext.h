#ifndef _RENDER_CONTEXT_H_
#define _RENDER_CONTEXT_H_


#include <grtypes.h>

class RenderContext
{
public:
    RenderContext()
    {
        m_Initialized = false;
        m_NativeContext = NULL;
    }
    virtual ~RenderContext() { }

    // Enable the context. All subsequent rendering commands will go here.
    virtual void setCurrent() { }
    // This is called before another context is made current. By default,
    // nothing is done here.
    virtual void endCurrent() { }

    virtual bool create(pvoid native, bool fullscreen = false, uint32 MSAASamples = 1, uint32 redBits = 8, uint32 greenBits = 8, uint32 blueBits = 8, uint32 alphaBits = 8, 
        uint32 depthBits = 24, uint32 stencilBits = 8, bool doubleBuffer = true) = 0;
    virtual void destroy() = 0;
        
    bool getInitialized() { return m_Initialized; };
    void setInitialized() { m_Initialized = true; };

    // Create a new context based on the same window/pbuffer as this
	//	context - mostly useful for additional threads.
	//  note The caller is responsible for deleting the returned context.
	virtual RenderContext* clone() { }
protected:
    bool    m_Initialized;
    // Window system native device that create this ctx;
    // Winodws GL: HDC; EGL/GLX: Dpy; DX: HWND.
    pvoid   m_NativeContext;
    // renderCtx
    // Windows GL: Hglrc; DX: D3DDevice
    pvoid   m_hCtx;
};

#endif

