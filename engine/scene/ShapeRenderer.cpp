// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <cassert>
#include <cmath>
#include "ShapeRenderer.hpp"
#include "Camera.hpp"
#include "../core/Engine.hpp"
#include "../graphics/Renderer.hpp"
#include "../utils/Utils.hpp"

namespace ouzel
{
    namespace scene
    {
        ShapeRenderer::ShapeRenderer():
            shader(engine->getCache().getShader(shaderColor)),
            blendState(engine->getCache().getBlendState(blendAlpha)),
            indexBuffer(*engine->getRenderer(),
                        graphics::BufferType::index,
                        graphics::Flags::dynamic),
            vertexBuffer(*engine->getRenderer(),
                         graphics::BufferType::vertex,
                         graphics::Flags::dynamic)
        {
        }

        void ShapeRenderer::draw(const Matrix4F& transformMatrix,
                                 float opacity,
                                 const Matrix4F& renderViewProjection,
                                 bool wireframe)
        {
            Component::draw(transformMatrix,
                            opacity,
                            renderViewProjection,
                            wireframe);

            if (dirty)
            {
                if (!indices.empty()) indexBuffer.setData(indices.data(), static_cast<std::uint32_t>(getVectorSize(indices)));
                if (!vertices.empty()) vertexBuffer.setData(vertices.data(), static_cast<std::uint32_t>(getVectorSize(vertices)));
                dirty = false;
            }

            const Matrix4F modelViewProj = renderViewProjection * transformMatrix;
            const float colorVector[] = {1.0F, 1.0F, 1.0F, opacity};

            for (const DrawCommand& drawCommand : drawCommands)
            {
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
                engine->getRenderer()->draw(indexBuffer.getResource(),
                                            drawCommand.indexCount,
                                            sizeof(std::uint16_t),
                                            vertexBuffer.getResource(),
                                            drawCommand.mode,
                                            drawCommand.startIndex);
            }
        }

        void ShapeRenderer::clear()
        {
            boundingBox.reset();

            drawCommands.clear();
            indices.clear();
            vertices.clear();

            dirty = true;
        }

        void ShapeRenderer::line(const Vector2F& start, const Vector2F& finish, Color color, float thickness)
        {
            assert(thickness >= 0.0F);

            DrawCommand command;
            command.startIndex = static_cast<std::uint32_t>(indices.size());

            auto startVertex = static_cast<std::uint16_t>(vertices.size());

            if (thickness == 0.0F)
            {
                command.mode = graphics::DrawMode::lineList;

                vertices.emplace_back(Vector3F(start), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F(finish), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                command.indexCount = 2;

                indices.push_back(startVertex + 0);
                indices.push_back(startVertex + 1);

                boundingBox.insertPoint(Vector3F(start));
                boundingBox.insertPoint(Vector3F(finish));
            }
            else
            {
                command.mode = graphics::DrawMode::triangleList;

                Vector2F tangent = finish - start;
                tangent.normalize();
                const Vector2F normal(-tangent.v[1], tangent.v[0]);

                const float halfThickness = thickness / 2.0F;

                vertices.emplace_back(Vector3F{start - tangent * halfThickness - normal * halfThickness},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{finish + tangent * halfThickness - normal * halfThickness},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{start - tangent * halfThickness + normal * halfThickness},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{finish + tangent * halfThickness + normal * halfThickness},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                command.indexCount = 6;

                indices.push_back(startVertex + 0);
                indices.push_back(startVertex + 1);
                indices.push_back(startVertex + 2);
                indices.push_back(startVertex + 1);
                indices.push_back(startVertex + 3);
                indices.push_back(startVertex + 2);

                boundingBox.insertPoint(vertices[startVertex + 0].position);
                boundingBox.insertPoint(vertices[startVertex + 1].position);
                boundingBox.insertPoint(vertices[startVertex + 2].position);
                boundingBox.insertPoint(vertices[startVertex + 3].position);
            }

            drawCommands.push_back(command);

            dirty = true;
        }

        void ShapeRenderer::circle(const Vector2F& position,
                                   float radius,
                                   Color color,
                                   bool fill,
                                   std::uint32_t segments,
                                   float thickness)
        {
            assert(radius >= 0.0F);
            assert(segments >= 3);
            assert(thickness >= 0.0F);

            DrawCommand command;
            command.startIndex = static_cast<std::uint32_t>(indices.size());

            const auto startVertex = static_cast<std::uint16_t>(vertices.size());

            if (fill)
            {
                command.mode = graphics::DrawMode::triangleStrip;

                vertices.emplace_back(Vector3F(position), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F}); // center

                for (std::uint32_t i = 0; i <= segments; ++i)
                    vertices.emplace_back(Vector3F{position.v[0] + radius * std::cos(i * tau<float> / static_cast<float>(segments)), position.v[1] + radius * std::sin(i * tau<float> / static_cast<float>(segments)), 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                command.indexCount = segments * 2 + 1;

                for (std::uint16_t i = 0; i < segments; ++i)
                {
                    indices.push_back(startVertex + i + 1);
                    indices.push_back(startVertex); // center
                }

                indices.push_back(startVertex + 1);

                boundingBox.insertPoint(Vector3F(position - Vector2F{radius, radius}));
                boundingBox.insertPoint(Vector3F(position + Vector2F{radius, radius}));
            }
            else
            {
                if (thickness == 0.0F)
                {
                    command.mode = graphics::DrawMode::lineStrip;

                    for (std::uint32_t i = 0; i <= segments; ++i)
                    {
                        vertices.emplace_back(Vector3F{position.v[0] + radius * std::cos(i * tau<float> / static_cast<float>(segments)), position.v[1] + radius * std::sin(i * tau<float> / static_cast<float>(segments)), 0.0F},
                                              color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    }

                    command.indexCount = segments + 1;

                    for (std::uint16_t i = 0; i < segments; ++i)
                        indices.push_back(startVertex + i);

                    indices.push_back(startVertex);

                    boundingBox.insertPoint(Vector3F(position - Vector2F{radius, radius}));
                    boundingBox.insertPoint(Vector3F(position + Vector2F{radius, radius}));
                }
                else
                {
                    command.mode = graphics::DrawMode::triangleStrip;

                    const float halfThickness = thickness / 2.0F;

                    for (std::uint32_t i = 0; i <= segments; ++i)
                    {
                        vertices.emplace_back(Vector3F{position.v[0] + (radius - halfThickness) * std::cos(i * tau<float> / static_cast<float>(segments)), position.v[1] + (radius - halfThickness) * std::sin(i * tau<float> / static_cast<float>(segments)), 0.0F},
                                              color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                        vertices.emplace_back(Vector3F{position.v[0] + (radius + halfThickness) * std::cos(i * tau<float> / static_cast<float>(segments)), position.v[1] + (radius + halfThickness) * std::sin(i * tau<float> / static_cast<float>(segments)), 0.0F},
                                              color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    }

                    for (const graphics::Vertex& vertex : vertices)
                        boundingBox.insertPoint(vertex.position);

                    command.indexCount = segments * 6;

                    for (std::uint16_t i = 0; i < segments; ++i)
                    {
                        if (i < segments - 1)
                        {
                            indices.push_back(startVertex + i * 2 + 0);
                            indices.push_back(startVertex + i * 2 + 1);
                            indices.push_back(startVertex + i * 2 + 3);

                            indices.push_back(startVertex + i * 2 + 3);
                            indices.push_back(startVertex + i * 2 + 2);
                            indices.push_back(startVertex + i * 2 + 0);
                        }
                        else
                        {
                            indices.push_back(startVertex + i * 2 + 0);
                            indices.push_back(startVertex + i * 2 + 1);
                            indices.push_back(startVertex + 1);

                            indices.push_back(startVertex + 1);
                            indices.push_back(startVertex + 0);
                            indices.push_back(startVertex + i * 2 + 0);
                        }
                    }
                }
            }

            drawCommands.push_back(command);

            dirty = true;
        }

        void ShapeRenderer::rectangle(const RectF& rectangle,
                                      Color color,
                                      bool fill,
                                      float thickness)
        {
            assert(thickness >= 0.0F);

            DrawCommand command;
            command.startIndex = static_cast<std::uint32_t>(indices.size());

            const auto startVertex = static_cast<std::uint16_t>(vertices.size());

            if (fill)
            {
                command.mode = graphics::DrawMode::triangleList;

                vertices.emplace_back(Vector3F{rectangle.left(), rectangle.bottom(), 0.0F},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{rectangle.right(), rectangle.bottom(), 0.0F},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{rectangle.right(), rectangle.top(), 0.0F},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                vertices.emplace_back(Vector3F{rectangle.left(), rectangle.top(), 0.0F},
                                      color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                command.indexCount = 6;

                indices.push_back(startVertex + 0);
                indices.push_back(startVertex + 1);
                indices.push_back(startVertex + 3);
                indices.push_back(startVertex + 1);
                indices.push_back(startVertex + 2);
                indices.push_back(startVertex + 3);

                boundingBox.insertPoint(Vector3F(rectangle.bottomLeft()));
                boundingBox.insertPoint(Vector3F(rectangle.topRight()));
            }
            else
            {
                if (thickness == 0.0F)
                {
                    command.mode = graphics::DrawMode::lineStrip;

                    // left bottom
                    vertices.emplace_back(Vector3F(rectangle.left(), rectangle.bottom(), 0.0F),
                                          color, Vector2F(), Vector3F(0.0F, 0.0F, -1.0F));

                    // right bottom
                    vertices.emplace_back(Vector3F(rectangle.right(), rectangle.bottom(), 0.0F),
                                          color, Vector2F(), Vector3F(0.0F, 0.0F, -1.0F));

                    // right top
                    vertices.emplace_back(Vector3F(rectangle.right(), rectangle.top(), 0.0F),
                                          color, Vector2F(), Vector3F(0.0F, 0.0F, -1.0F));

                    // left top
                    vertices.emplace_back(Vector3F(rectangle.left(), rectangle.top(), 0.0F),
                                          color, Vector2F(), Vector3F(0.0F, 0.0F, -1.0F));

                    command.indexCount = 5;

                    indices.push_back(startVertex + 0);
                    indices.push_back(startVertex + 1);
                    indices.push_back(startVertex + 2);
                    indices.push_back(startVertex + 3);
                    indices.push_back(startVertex + 0);

                    boundingBox.insertPoint(Vector3F(rectangle.bottomLeft()));
                    boundingBox.insertPoint(Vector3F(rectangle.topRight()));
                }
                else
                {
                    command.mode = graphics::DrawMode::triangleList;

                    const float halfThickness = thickness / 2.0F;

                    // left bottom
                    vertices.emplace_back(Vector3F{rectangle.left() - halfThickness, rectangle.bottom() - halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    vertices.emplace_back(Vector3F{rectangle.left() + halfThickness, rectangle.bottom() + halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                    // right bottom
                    vertices.emplace_back(Vector3F{rectangle.right() + halfThickness, rectangle.bottom() - halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    vertices.emplace_back(Vector3F{rectangle.right() - halfThickness, rectangle.bottom() + halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                    // right top
                    vertices.emplace_back(Vector3F{rectangle.right() + halfThickness, rectangle.top() + halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    vertices.emplace_back(Vector3F{rectangle.right() - halfThickness, rectangle.top() - halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                    // left top
                    vertices.emplace_back(Vector3F{rectangle.left() - halfThickness, rectangle.top() + halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                    vertices.emplace_back(Vector3F{rectangle.left() + halfThickness, rectangle.top() - halfThickness, 0.0F},
                                          color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                    command.indexCount = 24;
                    // bottom
                    indices.push_back(startVertex + 0);
                    indices.push_back(startVertex + 2);
                    indices.push_back(startVertex + 1);

                    indices.push_back(startVertex + 2);
                    indices.push_back(startVertex + 3);
                    indices.push_back(startVertex + 1);

                    // right
                    indices.push_back(startVertex + 2);
                    indices.push_back(startVertex + 4);
                    indices.push_back(startVertex + 3);

                    indices.push_back(startVertex + 4);
                    indices.push_back(startVertex + 5);
                    indices.push_back(startVertex + 3);

                    // top
                    indices.push_back(startVertex + 4);
                    indices.push_back(startVertex + 6);
                    indices.push_back(startVertex + 5);

                    indices.push_back(startVertex + 6);
                    indices.push_back(startVertex + 7);
                    indices.push_back(startVertex + 5);

                    // left
                    indices.push_back(startVertex + 6);
                    indices.push_back(startVertex + 0);
                    indices.push_back(startVertex + 7);

                    indices.push_back(startVertex + 0);
                    indices.push_back(startVertex + 1);
                    indices.push_back(startVertex + 7);

                    boundingBox.insertPoint(Vector3F(rectangle.bottomLeft() - Vector2F(halfThickness, halfThickness)));
                    boundingBox.insertPoint(Vector3F(rectangle.topRight() + Vector2F(halfThickness, halfThickness)));
                }
            }

            drawCommands.push_back(command);

            dirty = true;
        }

        void ShapeRenderer::polygon(const std::vector<Vector2F>& edges,
                                    Color color,
                                    bool fill,
                                    float thickness)
        {
            assert(edges.size() >= 3);
            assert(thickness >= 0.0F);

            DrawCommand command;
            command.startIndex = static_cast<std::uint32_t>(indices.size());

            const auto startVertex = static_cast<std::uint16_t>(vertices.size());

            if (fill)
            {
                command.mode = graphics::DrawMode::triangleList;

                for (std::uint16_t i = 0; i < edges.size(); ++i)
                    vertices.emplace_back(Vector3F(edges[i]), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                command.indexCount = static_cast<std::uint32_t>(edges.size() - 2) * 3;

                for (std::uint16_t i = 1; i < edges.size() - 1; ++i)
                {
                    indices.push_back(startVertex);
                    indices.push_back(startVertex + i);
                    indices.push_back(startVertex + i + 1);
                }

                for (std::uint16_t i = 0; i < edges.size(); ++i)
                    boundingBox.insertPoint(Vector3F(edges[i]));
            }
            else
            {
                if (thickness == 0.0F)
                {
                    command.mode = graphics::DrawMode::lineStrip;

                    for (std::uint16_t i = 0; i < edges.size(); ++i)
                        vertices.emplace_back(Vector3F(edges[i]), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                    command.indexCount = static_cast<std::uint32_t>(edges.size()) + 1;

                    for (std::uint16_t i = 0; i < edges.size(); ++i)
                        indices.push_back(startVertex + i);

                    indices.push_back(startVertex);

                    for (std::uint16_t i = 0; i < edges.size(); ++i)
                        boundingBox.insertPoint(Vector3F(edges[i]));
                }
                else
                {
                    // TODO: implement
                }
            }

            drawCommands.push_back(command);

            dirty = true;
        }

        namespace
        {
            std::vector<std::uint32_t> pascalsTriangleRow(std::uint32_t row)
            {
                std::vector<std::uint32_t> ret;
                ret.push_back(1);
                for (std::uint32_t i = 0; i < row; ++i)
                    ret.push_back(ret[i] * (row - i) / (i + 1));

                return ret;
            }
        }

        void ShapeRenderer::curve(const std::vector<Vector2F>& controlPoints,
                                  Color color,
                                  std::uint32_t segments,
                                  float thickness)
        {
            assert(controlPoints.size() >= 2);
            assert(thickness >= 0.0F);

            DrawCommand command;
            command.startIndex = static_cast<std::uint32_t>(indices.size());

            const auto startVertex = static_cast<std::uint16_t>(vertices.size());

            if (thickness == 0.0F)
            {
                command.mode = graphics::DrawMode::lineStrip;
                command.indexCount = 0;

                if (controlPoints.size() == 2)
                {
                    for (std::uint16_t i = 0; i < controlPoints.size(); ++i)
                    {
                        indices.push_back(startVertex + static_cast<std::uint16_t>(command.indexCount));
                        ++command.indexCount;
                        vertices.emplace_back(Vector3F(controlPoints[i]), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});
                        boundingBox.insertPoint(Vector3F(controlPoints[i]));
                    }
                }
                else
                {
                    const std::vector<std::uint32_t> binomialCoefficients = pascalsTriangleRow(static_cast<std::uint32_t>(controlPoints.size() - 1));

                    for (std::uint32_t segment = 0; segment < segments; ++segment)
                    {
                        auto t = static_cast<float>(segment) / static_cast<float>(segments - 1);
                        Vector2F position;

                        for (std::uint16_t n = 0; n < controlPoints.size(); ++n)
                            position += static_cast<float>(binomialCoefficients[n]) *
                                powf(t, static_cast<float>(n)) *
                                powf(1.0F - t, static_cast<float>(controlPoints.size() - n - 1)) *
                                controlPoints[n];

                        graphics::Vertex vertex(Vector3F(position), color, Vector2F(), Vector3F{0.0F, 0.0F, -1.0F});

                        indices.push_back(startVertex + static_cast<std::uint16_t>(command.indexCount));
                        ++command.indexCount;
                        vertices.push_back(vertex);
                        boundingBox.insertPoint(vertex.position);
                    }
                }
            }
            else
            {
                // TODO: implement
            }

            drawCommands.push_back(command);

            dirty = true;
        }
    } // namespace scene
} // namespace ouzel
