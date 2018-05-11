/*  Panel rendering class

    Copyright (C) 2016 Arthur M
*/

#include <cairo/cairo.h>
#include <vector>

#include "IContainer.hpp"

#ifndef _GUI_PANEL
#define _GUI_PANEL

namespace Familyline {
namespace Graphics {
namespace GUI {

    struct PanelData {
	IPanel* panel;
	int x, y, w, h;

	cairo_t* ctxt;
	cairo_surface_t* csurf;
    };
    
/*
    A panel is the most basic window structore for Familyline's GUI
*/
class Panel : public IContainer {
private:
        
    std::vector<PanelData> _panels;
    IPanel* oldPanel = nullptr;
public:
    Panel();
    Panel(int x, int y, int w, int h);
    Panel(double x, double y, double w, double h, bool rel);

    virtual void Redraw(cairo_t* ctxt) override;

    /* Add a panel using the panel position or a new position */
    virtual int AddPanel(IPanel* p) override;
    virtual int AddPanel(IPanel* p, int x, int y) override;
    virtual int AddPanel(IPanel* p, double x, double y) override;
    
    /* Remove the panel */
    virtual void RemovePanel(IPanel* p) override;

    virtual bool ProcessInput(Input::InputEvent& ev) override;

    virtual void OnFocus() override {}
    virtual void OnLostFocus() override;

    virtual bool IsDirty() const override { return true; }

    void ClearChildren();

    virtual ~Panel();
};

}
}
}


#endif /* end of include guard: _GUI_WINDOW */
