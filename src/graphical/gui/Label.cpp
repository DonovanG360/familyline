#include "Label.hpp"

using namespace Tribalia::Graphics::GUI;

Label::Label(int x, int y, const char* text)
{
    _text = std::string{text};
    _xPos = x;
    _yPos = y;
    _width = _text.size() * 9;
    _height = 16.0f;
    _bgColor = glm::vec4(255, 255, 255, 0);
    
}

Label::Label(int x, int y, int w, int h, const char* text)
    : IPanel(x,y,w,h)
{
    _text = std::string{text};
    _bgColor = glm::vec4(255, 255, 255, 0);
}

const char* Label::GetText() const
{
    return _text.c_str();
}

void Label::SetText(char* txt, ...)
{
    va_list vl;
    va_start(vl, txt);
    char* s = new char[512+strlen(txt)];
    vsprintf(s, txt, vl);
    va_end(vl);

    _text = std::string{s};
}


void Label::Redraw(cairo_t* ctxt)
{
    /* Redraw the text */
    cairo_set_source_rgba(ctxt, _fgColor.r, _fgColor.g, _fgColor.b, _fgColor.a);
    cairo_select_font_face(ctxt, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

    /* Draw the text relative to container.
       The position only interests for our parent know where to render us 
    */
    cairo_set_font_size(ctxt, 14.0);
    cairo_move_to(ctxt, 0, 14);
    cairo_show_text(ctxt, _text.c_str());
}
