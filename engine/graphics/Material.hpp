// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_MATERIAL_HPP
#define OUZEL_GRAPHICS_MATERIAL_HPP

#include <memory>
#include "Renderer.hpp"
#include "BlendState.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "../math/Color.hpp"

namespace ouzel
{
    namespace graphics
    {
        class Material final
        {
        public:
            static constexpr std::uint32_t TEXTURE_LAYERS = 4;

            Material() = default;

            Material(const Material&) = delete;
            Material& operator=(const Material&) = delete;

            Material(Material&&) = delete;
            Material& operator=(Material&&) = delete;

            const BlendState* blendState = nullptr;
            const Shader* shader = nullptr;
            std::shared_ptr<Texture> textures[TEXTURE_LAYERS];
            CullMode cullMode = CullMode::back;
            Color diffuseColor = Color::white();
            float opacity = 1.0F;
        };
    } // namespace graphics
} // namespace ouzel

#endif // OUZEL_GRAPHICS_MATERIAL_HPP
