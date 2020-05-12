// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "TextRenderer.hpp"
#include "Camera.hpp"
#include "../core/Engine.hpp"
#include "../graphics/Renderer.hpp"
#include "../assets/Cache.hpp"
#include "../utils/Utils.hpp"

namespace ouzel
{
    namespace scene
    {
        TextRenderer::TextRenderer(const std::string& fontFile,
                                   float initFontSize,
                                   const std::string& initText,
                                   Color initColor,
                                   const Vector2F& initTextAnchor):
            shader(engine->getCache().getShader(SHADER_TEXTURE)),
            blendState(engine->getCache().getBlendState(BLEND_ALPHA)),
            indexBuffer(*engine->getRenderer(),
                        graphics::BufferType::index,
                        graphics::Flags::dynamic),
            vertexBuffer(*engine->getRenderer(),
                         graphics::BufferType::vertex,
                         graphics::Flags::dynamic),
            text(initText),
            fontSize(initFontSize),
            textAnchor(initTextAnchor),
            color(initColor)
        {
            whitePixelTexture = engine->getCache().getTexture(TEXTURE_WHITE_PIXEL);

            font = engine->getCache().getFont(fontFile);

            updateText();
        }

        void TextRenderer::setFont(const std::string& fontFile)
        {
            font = engine->getCache().getFont(fontFile);

            updateText();
        }

        void TextRenderer::setTextAnchor(const Vector2F& newTextAnchor)
        {
            textAnchor = newTextAnchor;

            updateText();
        }

        void TextRenderer::setFontSize(float newFontSize)
        {
            fontSize = newFontSize;

            updateText();
        }

        void TextRenderer::draw(const Matrix4F& transformMatrix,
                                float opacity,
                                const Matrix4F& renderViewProjection,
                                bool wireframe)
        {
            Component::draw(transformMatrix,
                            opacity,
                            renderViewProjection,
                            wireframe);

            if (needsMeshUpdate)
            {
                indexBuffer.setData(indices.data(), static_cast<std::uint32_t>(getVectorSize(indices)));
                vertexBuffer.setData(vertices.data(), static_cast<std::uint32_t>(getVectorSize(vertices)));

                needsMeshUpdate = false;
            }

            const Matrix4F modelViewProj = renderViewProjection * transformMatrix;
            const float colorVector[] = {color.normR(), color.normG(), color.normB(), color.normA() * opacity};

            std::vector<std::vector<float>> fragmentShaderConstants(1);
            fragmentShaderConstants[0] = {std::begin(colorVector), std::end(colorVector)};

            std::vector<std::vector<float>> vertexShaderConstants(1);
            vertexShaderConstants[0] = {std::begin(modelViewProj.m), std::end(modelViewProj.m)};

            engine->getRenderer()->setPipelineState(blendState->getResource(),
                                                    shader->getResource(),
                                                    graphics::CullMode::none,
                                                    wireframe ? graphics::FillMode::wireframe : graphics::FillMode::solid);
            engine->getRenderer()->setShaderConstants(fragmentShaderConstants,
                                                      vertexShaderConstants);
            engine->getRenderer()->setTextures({wireframe ? whitePixelTexture->getResource() : texture->getResource()});
            engine->getRenderer()->draw(indexBuffer.getResource(),
                                        static_cast<std::uint32_t>(indices.size()),
                                        sizeof(std::uint16_t),
                                        vertexBuffer.getResource(),
                                        graphics::DrawMode::triangleList,
                                        0);
        }

        void TextRenderer::setText(const std::string& newText)
        {
            text = newText;

            updateText();
        }

        void TextRenderer::setColor(Color newColor)
        {
            color = newColor;
        }

        void TextRenderer::updateText()
        {
            boundingBox.reset();

            if (font)
            {
                std::tie(indices, vertices, texture) = font->getRenderData(text, Color::white(), fontSize, textAnchor);
                needsMeshUpdate = true;

                for (const graphics::Vertex& vertex : vertices)
                    boundingBox.insertPoint(vertex.position);
            }
            else
            {
                indices.clear();
                vertices.clear();
                texture.reset();
            }
        }
    } // namespace scene
} // namespace ouzel
