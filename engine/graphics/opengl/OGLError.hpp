// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_OGLERROR_HPP
#define OUZEL_GRAPHICS_OGLERROR_HPP

#include <stdexcept>

namespace ouzel::graphics::opengl
{
    class Error final: public std::runtime_error
    {
    public:
        explicit Error(const std::string& str): std::runtime_error(str) {}
        explicit Error(const char* str): std::runtime_error(str) {}
    };
}

#endif // OUZEL_GRAPHICS_OGLERROR_HPP
