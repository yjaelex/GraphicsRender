
#include "RenderWindow.h"

RenderWindow::RenderWindow()
{
    m_IsFullScreen = false;
    m_Width = 0;
    m_Height = 0;
    m_doubleBuffer = false;
    m_VSyncInterval = 1;
    m_VsyncEnable = false;
    m_Active = false;
	m_Closed = false;
    m_Hidden = false;
}

void RenderWindow::getMetrics(unsigned int& width, unsigned int& height, unsigned int& colourDepth)
{
    width = m_Width;
    height = m_Height;
    colourDepth = 32;
}

void RenderWindow::parseParam(const NameValuePairList * miscParams)
{
    if (miscParams==NULL) return;

    NameValuePairList::iterator iter;
    for(iter = miscParams->begin(); iter != miscParams->end(); iter++)
    {
        if (iter->first == "vsync")
        {
            m_VsyncEnable = iter->second;
        }
        else if (iter->first == "vsyncInterval")
        {
            m_VSyncInterval = iter->second;
        }
    }

}