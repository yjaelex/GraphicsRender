#ifndef _GL_RENDER_CONTEXT_H_
#define _GL_RENDER_CONTEXT_H_

#include "RenderContext.h"

#ifdef _WIN32
#include <Windows.h>
#include "GL/glew.h"
#include "GL/wglew.h"
#include <GL/glu.h>

class WGLRenderContext : public RenderContext
{
public:
    WGLRenderContext() { }

    // Create an external GL Ctx.
    WGLRenderContext(HDC hDC, HGLRC glrc)
    {
        m_NativeContext = hDC;
        m_HDC = hDC;
        m_Glrc = glrc;
        m_hCtx = glrc;
        setInitialized();
    }

    virtual ~WGLRenderContext() 
    { 
        if(m_Glrc)   destroy();
    }

    virtual void setCurrent();
    virtual void endCurrent();
    virtual RenderContext* clone();

    virtual bool create(pvoid native, bool fullscreen = false, uint32 MSAASamples = 1, uint32 redBits = 8, uint32 greenBits = 8, uint32 blueBits = 8, uint32 alphaBits = 8, 
        uint32 depthBits = 24, uint32 stencilBits = 8, bool doubleBuffer = true);
    virtual void destroy();

private:
    HDC     m_HDC;
    HGLRC   m_Glrc;
}

#else
#ifdef LINUX
class GLXRenderContext : public RenderContext
{


}

#endif
#endif


#endif


