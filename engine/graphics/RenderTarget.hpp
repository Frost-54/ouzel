// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_RENDERTARGET_HPP
#define OUZEL_GRAPHICS_RENDERTARGET_HPP

#include <memory>
#include <vector>
#include "RenderDevice.hpp"
#include "../math/Color.hpp"
#include "../math/Size.hpp"

namespace ouzel::graphics
{
    class Graphics;
    class Texture;

    class RenderTarget final
    {
    public:
        RenderTarget() = default;

        RenderTarget(Graphics& initGraphics,
                     const std::vector<Texture*>& initColorTextures,
                     Texture* initDepthTexture);

        auto& getResource() const noexcept { return resource; }

        auto& getColorTextures() const noexcept { return colorTextures; }
        auto getDepthTexture() const noexcept { return depthTexture; }

    private:
        RenderDevice::Resource resource;

        std::vector<Texture*> colorTextures;
        Texture* depthTexture = nullptr;
    };
}

#endif // OUZEL_GRAPHICS_RENDERTARGET_HPP
