#include "Cursor.hpp"

using namespace Familyline::Input;

Cursor* Cursor::cursor = nullptr;

void Cursor::GetPositions(int& x, int& y)
{
    InputEvent ev;
    if (_listener->PopEvent(ev)) {
        _x = ev.mousex;
        _y = ev.mousey;
	_listener->SetAccept();
    }

    x = _x;
    y = _y;
}

Cursor::Cursor()
{
    _listener = new InputListener{"cursor"};
    InputManager::GetInstance()->AddListener(EVENT_MOUSEMOVE, _listener, 0.01);
}
