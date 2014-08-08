
#include "BaseApp.h"

BaseApp::BaseApp()
{
    width = fullscreenWidth = 800;
    height = fullscreenHeight = 600;
    colorBits = 32;
    depthBits = 24;
    stencilBits = 8;
    fullscreen = showFPS = vSync = false;
    title.assign("Graphics Render Demo.");
}
