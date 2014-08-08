#ifndef _BASEAPP_H_
#define _BASEAPP_H_

#ifdef _WIN32
#include <windows.h>
#else
#ifdef LINUX
#include <GL/glx.h>
#endif
#endif

#include <grtypes.h>
#include <string>

using namespace std;

class BaseApp 
{
public:
	BaseApp();
	virtual ~BaseApp();

    virtual initApp();
    virtual exitApp();
    virtual run();

private:
	int32 width, height, fullscreenWidth, fullscreenHeight, screen;
	int32 colorBits, depthBits, stencilBits;

	int32 antiAliasSamples;
	bool fullscreen;

	bool showFPS;
	bool vSync;
}

#endif
