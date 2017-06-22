/*  Interface for a panel

    Copyright (C) 2016 Arthur M
*/

#include "IControl.hpp"
#include <cstdint>
#include <glm/glm.hpp>

#ifndef _GUI_IPANEL
#define _GUI_IPANEL

namespace Tribalia {
namespace Graphics {
namespace GUI {

/*
    Interface for implementing a panel

    This is here to ease treating GUIRenderer like a panel, so we can
    easily implement redrawing without
    circular references

    Panels usually have positions and sizes
*/
class IPanel : public IControl {
protected:
       
    int _xPos, _yPos;
    int _width, _height;
    double _zindex = 0.5;

    double _fxPos = 2.0, _fyPos = 2.0;
    double _fwidth = 640.0, _fheight = 480.0;

    /* Resize the panel p according to the absolute size of the 'this'
       panel (i.e, uses width and height of this panel to set the position
       of 'p' panel */
    void ResizePanelAbsolute(IPanel* p);

    
public:
    IPanel();
    IPanel(int x, int y, int w, int h);
    IPanel(double x, double y, double w, double h, bool relative);

    void SetBounds(int x, int y, int w, int h);
    void SetPosition(int x, int y);
    void GetBounds(int& x, int& y, int& w, int& h) const;
    
    void SetBounds(double x, double y, double w, double h);
    void SetPosition(double x, double y);
    void GetBounds(double& x, double& y, double& w, double& h) const;

    void SetZIndex(double);
    double GetZIndex() const;

    /*  Non-virtual functions in an interface?
        C++ allows this. I won't implement this on every control I make. */
    int GetBackColor() const;
    int GetBackColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;
    void SetBackColor(int rgba);
    void SetBackColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a);

    int GetForeColor() const;
    int GetForeColor(uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) const;
    void SetForeColor(int rgba);
    void SetForeColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
    
    virtual ~IPanel(){}
};

}
}
}


#endif /* end of include guard: _GUI_WINDOW */
