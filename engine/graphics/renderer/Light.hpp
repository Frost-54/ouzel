// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_RENDERER_LIGHT_HPP
#define OUZEL_GRAPHICS_RENDERER_LIGHT_HPP

#include "Renderer.hpp"
#include "../../math/Matrix.hpp"

namespace ouzel::graphics::renderer
{
    class Light final
    {
    public:
        enum class Type
        {
            point,
            spot,
            directional
        };

        Light(Renderer& initRenderer):
            renderer{initRenderer},
            resource{initRenderer}
        {
        }

    private:
        Renderer& renderer;
        Renderer::Resource resource;
        Matrix4F transform;
    };
}

#endif // OUZEL_GRAPHICS_RENDERER_LIGHT_HPP
