/***
    Class that contains all graphical exceptions

    Copyright (C) 2016 Arthur M

***/

#include <stdexcept>

#ifndef GFXEXCEPTIONS_HPP
#define GFXEXCEPTIONS_HPP

namespace Tribalia {
namespace Graphics {

    class renderer_exception : public std::runtime_error
    {
    public:
        explicit renderer_exception(std::string msg, int code);
        int code;
    };


} /* Graphics */
} /* Tribalia */



#endif /* end of include guard: GFXEXCEPTIONS_HPP */
