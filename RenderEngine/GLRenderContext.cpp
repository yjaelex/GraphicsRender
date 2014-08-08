#include "GLRenderContext.h"

#ifdef _WIN32

void WGLRenderContext::setCurrent()
{
    wglMakeCurrent(m_HDC, m_Glrc);      
}
void WGLRenderContext::endCurrent()
{
    wglMakeCurrent(NULL, NULL);
}

void WGLRenderContext::destroy()
{
    if (m_Glrc != NULL)
    {
        wglDeleteContext(m_Glrc);
        m_Glrc = NULL;
        m_HDC  = NULL;
    }		
}

bool WGLRenderContext::create(pvoid native, bool fullscreen = false, uint32 MSAASamples = 1, uint32 redBits = 8, uint32 greenBits = 8, uint32 blueBits = 8, uint32 alphaBits = 8, 
        uint32 depthBits = 24, uint32 stencilBits = 8, bool doubleBuffer = true)
{
    m_NativeContext = native;
    m_HDC = (HDC)native;

    int32  PixelFormat;
    uint32  bits = redBits+greenBits+blueBits;
    PIXELFORMATDESCRIPTOR pfd=				        // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		doubleBuffer ? PFD_DOUBLEBUFFER : 0,	    // Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		alphaBits,								    // Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		depthBits,								    // Depth Buffer) 
		stencilBits,							    // Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

    if (MSAASamples==1)
    {
	    if (!(PixelFormat=ChoosePixelFormat(m_HDC,&pfd)))	// Did Windows Find A Matching Pixel Format?
	    {
		    // (NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		    return false;								// Return FALSE
	    }
    }
    else
    {
        bool valid;
        uint32 numFormats;
        float32 fAttributes[] = {0,0};
 
        int32 iAttributes[] = { WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
            WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
            WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
            WGL_COLOR_BITS_ARB,bits,
            WGL_ALPHA_BITS_ARB,alphaBits,
            WGL_DEPTH_BITS_ARB,depthBits,
            WGL_STENCIL_BITS_ARB,stencilBits,
            WGL_DOUBLE_BUFFER_ARB,doubleBuffer,
            WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
            WGL_SAMPLES_ARB, MSAASamples , 
            0,0};
 
        // First We Check To See If We Can Get A Pixel Format For 4 Samples
        valid = wglChoosePixelFormatARB(m_HDC,iAttributes,fAttributes,1,&PixelFormat,&numFormats);
  
        // if We Returned True, And Our Format Count Is Greater Than 1
        if (valid && numFormats >= 1)
        {
            // MSAA samples not supported.
            return false;
        }
    }

	if(!SetPixelFormat(m_HDC,PixelFormat,&pfd))		// Are We Able To Set The Pixel Format?
	{
		// (NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

	if (!(m_Glrc=wglCreateContext(m_HDC)))				// Are We Able To Get A Rendering Context?
	{
		// (NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;								// Return FALSE
	}

    m_hCtx = m_Glrc;
}

RenderContext* WGLRenderContext::clone()
{
    // Create new context based on own HDC
    HGLRC newCtx = wglCreateContext(m_HDC);

    if (!newCtx)
    {
	    // "Can't Create A GL Rendering Context.","ERROR" "Win32Context::clone"
        return NULL;
    }

    HGLRC oldrc = wglGetCurrentContext();
    HDC oldhdc = wglGetCurrentDC();
    wglMakeCurrent(NULL, NULL);
    // Share lists with old context
    if (!wglShareLists(m_Glrc, newCtx))
    {
        // String("wglShareLists() failed. Win32Context::clone"
	    wglDeleteContext(newCtx);
        return NULL;
    }
    // restore old context
    wglMakeCurrent(oldhdc, oldrc);

    return new WGLRenderContext(m_HDC, newCtx);
}

#endif
