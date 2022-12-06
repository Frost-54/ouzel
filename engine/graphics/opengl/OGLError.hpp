// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_OGLERROR_HPP
#define OUZEL_GRAPHICS_OGLERROR_HPP

#include <stdexcept>

namespace ouzel::graphics::opengl
{
    class Error final: public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };
}

#endif // OUZEL_GRAPHICS_OGLERROR_HPP
