// Ouzel by Elviss Strazdins

#include <cassert>
#include <algorithm>
#include "Camera.hpp"
#include "Actor.hpp"
#include "Layer.hpp"
#include "../core/Engine.hpp"
#include "../graphics/Graphics.hpp"
#include "../graphics/RenderDevice.hpp"

namespace ouzel::scene
{
    Camera::Camera(const math::Matrix<float, 4>& initProjection):
        projectionMode{ProjectionMode::custom}, projection{initProjection}

    {
    }

    Camera::Camera(const math::Size<float, 2>& initTargetContentSize, ScaleMode initScaleMode):
        projectionMode{ProjectionMode::orthographic},
        targetContentSize{initTargetContentSize},
        scaleMode{initScaleMode}
    {
    }

    Camera::Camera(float initFov, float initNearPlane, float initFarPlane):
        projectionMode{ProjectionMode::perspective},
        fov{initFov},
        nearPlane{initNearPlane},
        farPlane{initFarPlane}
    {
    }

    Camera::~Camera()
    {
        if (layer) layer->removeCamera(*this);
    }

    void Camera::setActor(Actor* newActor)
    {
        Component::setActor(newActor);

        viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::setLayer(Layer* newLayer)
    {
        if (layer) layer->removeCamera(*this);

        Component::setLayer(newLayer);

        if (layer) layer->addCamera(*this);
    }

    void Camera::updateTransform()
    {
        Component::updateTransform();

        viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::calculateProjection() const
    {
        const math::Size<std::uint32_t, 2> renderTargetSize = renderTarget ?
            !renderTarget->getColorTextures().empty() ?
                renderTarget->getColorTextures()[0]->getSize() :
                renderTarget->getDepthTexture() ?
                    renderTarget->getDepthTexture()->getSize() :
                    math::Size<std::uint32_t, 2>{} :
            engine->getGraphics().getSize();

        renderViewport.position.v[0] = renderTargetSize.v[0] * viewport.position.v[0];
        renderViewport.position.v[1] = renderTargetSize.v[1] * viewport.position.v[1];
        renderViewport.size.v[0] = renderTargetSize.v[0] * viewport.size.v[0];
        renderViewport.size.v[1] = renderTargetSize.v[1] * viewport.size.v[1];

        assert(renderViewport.size.v[0] > 0.0F && renderViewport.size.v[1] > 0.0F);

        if (targetContentSize.v[0] > 0.0F && targetContentSize.v[1] > 0.0F)
        {
            contentScale.v[0] = renderViewport.size.v[0] / targetContentSize.v[0];
            contentScale.v[1] = renderViewport.size.v[1] / targetContentSize.v[1];

            switch (scaleMode)
            {
                case ScaleMode::noScale:
                {
                    break;
                }
                case ScaleMode::exactFit:
                {
                    contentScale.v[0] = 1.0F;
                    contentScale.v[1] = 1.0F;
                    break;
                }
                case ScaleMode::noBorder:
                {
                    contentScale.v[0] = contentScale.v[1] = std::max(contentScale.v[0], contentScale.v[1]);
                    break;
                }
                case ScaleMode::showAll:
                {
                    contentScale.v[0] = contentScale.v[1] = std::min(contentScale.v[0], contentScale.v[1]);
                    break;
                }
                default:
                    return;
            }

            contentSize = math::Size<float, 2>{
                renderViewport.size.v[0] / contentScale.v[0],
                renderViewport.size.v[1] / contentScale.v[1]
            };
            contentPosition = math::Vector<float, 2>{
                (contentSize.v[0] - targetContentSize.v[0]) / 2.0F,
                (contentSize.v[1] - targetContentSize.v[1]) / 2.0F
            };
        }
        else
        {
            contentScale = math::Vector<float, 2>{1.0F, 1.0F};
            contentSize = math::Size<float, 2>{renderViewport.size.v[0], renderViewport.size.v[1]};
            contentPosition = math::Vector<float, 2>{0.0F, 0.0F};
        }

        switch (projectionMode)
        {
            case ProjectionMode::custom:
                // do nothing
                break;
            case ProjectionMode::orthographic:
                setOrthographic(projection, contentSize.v[0], contentSize.v[1], -1.0F, 1.0F);
                break;
            case ProjectionMode::perspective:
                setPerspective(projection, fov, contentSize.v[0] / contentSize.v[1], nearPlane, farPlane);
                break;
            default:
                return;
        }

        projectionDirty = false;
        viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    const math::Matrix<float, 4>& Camera::getViewProjection() const
    {
        if (viewProjectionDirty) calculateViewProjection();

        return viewProjection;
    }

    const math::Matrix<float, 4>& Camera::getRenderViewProjection() const
    {
        if (viewProjectionDirty) calculateViewProjection();

        return renderViewProjection;
    }

    const math::Matrix<float, 4>& Camera::getInverseViewProjection() const
    {
        if (inverseViewProjectionDirty)
        {
            inverseViewProjection = getViewProjection();
            invert(inverseViewProjection);

            inverseViewProjectionDirty = false;
        }

        return inverseViewProjection;
    }

    void Camera::calculateViewProjection() const
    {
        if (projectionDirty) calculateProjection();

        if (actor)
        {
            viewProjection = projection * actor->getInverseTransform();

            renderViewProjection = engine->getGraphics().getDevice()->getProjectionTransform(renderTarget != nullptr) * viewProjection;

            viewProjectionDirty = false;
        }
    }

    math::Vector<float, 3> Camera::convertClipToWorld(const math::Vector<float, 3>& clipPosition) const noexcept
    {
        math::Vector<float, 3> result = clipPosition;
        transformPoint(getInverseViewProjection(), result);
        return result;
    }

    math::Vector<float, 3> Camera::convertWorldToClip(const math::Vector<float, 3>& worldPosition) const noexcept
    {
        math::Vector<float, 3> result = worldPosition;
        transformPoint(getViewProjection(), result);
        return result;
    }

    math::Vector<float, 3> Camera::convertNormalizedToWorld(const math::Vector<float, 2>& normalizedPosition) const noexcept
    {
        // convert window normalized to viewport clip position
        auto clipPosition = math::Vector<float, 3>{((normalizedPosition.v[0] - viewport.position.v[0]) / viewport.size.v[0] - 0.5F) * 2.0F,
                               (((1.0F - normalizedPosition.v[1]) - viewport.position.v[1]) / viewport.size.v[1] - 0.5F) * 2.0F,
                               0.0F};

        return convertClipToWorld(clipPosition);
    }

    math::Vector<float, 2> Camera::convertWorldToNormalized(const math::Vector<float, 3>& worldPosition) const noexcept
    {
        const auto clipPosition = convertWorldToClip(worldPosition);

        // convert viewport clip position to window normalized
        return math::Vector<float, 2>{
            (clipPosition.v[0] / 2.0F + 0.5F) * viewport.size.v[0] + viewport.position.v[0],
            1.0F - ((clipPosition.v[1] / 2.0F + 0.5F) * viewport.size.v[1] + viewport.position.v[1])
        };
    }

    bool Camera::checkVisibility(const math::Matrix<float, 4>& boxTransform, const math::Box<float, 3>& box) const
    {
        if (projectionMode == ProjectionMode::orthographic)
        {
            // calculate center point of the box
            const auto diff = math::Vector<float, 2>{box.max - box.min};

            // offset the center point, so that it is relative to 0,0
            math::Vector<float, 3> v3p{
                box.min.v[0] + diff.v[0] / 2.0F,
                box.min.v[1] + diff.v[1] / 2.0F, 0.0F
            };

            // apply local transform to the center point
            transformPoint(boxTransform, v3p);

            // tranform the center to viewport's clip space
            const auto clipPos = math::Vector<float, 4>{v3p.v[0], v3p.v[1], v3p.v[2], 1.0F} * getViewProjection();

            assert(clipPos.v[3] != 0.0F);

            // normalize position of the center point
            const math::Vector<float, 2> v2p{
                (clipPos.v[0] / clipPos.v[3] + 1.0F) * 0.5F,
                (clipPos.v[1] / clipPos.v[3] + 1.0F) * 0.5F
            };

            // calculate half size
            const math::Size<float, 2> halfSize{diff.v[0] / 2.0F, diff.v[1] / 2.0F};

            // convert content size to world coordinates
            math::Size<float, 2> halfWorldSize{
                std::max(std::fabs(halfSize.v[0] * boxTransform.m.v[0] + halfSize.v[1] * boxTransform.m.v[4]),
                         std::fabs(halfSize.v[0] * boxTransform.m.v[0] - halfSize.v[1] * boxTransform.m.v[4])),
                std::max(std::fabs(halfSize.v[0] * boxTransform.m.v[1] + halfSize.v[1] * boxTransform.m.v[5]),
                         std::fabs(halfSize.v[0] * boxTransform.m.v[1] - halfSize.v[1] * boxTransform.m.v[5]))
            };

            // scale half size by camera projection to get the size in clip space coordinates
            halfWorldSize.v[0] *= (std::fabs(viewProjection.m.v[0]) + std::fabs(viewProjection.m.v[4])) / 2.0F;
            halfWorldSize.v[1] *= (std::fabs(viewProjection.m.v[1]) + std::fabs(viewProjection.m.v[5])) / 2.0F;

            // create visible rect in clip space
            const math::Rect<float> visibleRect{
                -halfWorldSize.v[0],
                -halfWorldSize.v[1],
                1.0F + halfWorldSize.v[0] * 2.0F,
                1.0F + halfWorldSize.v[1] * 2.0F
            };

            return containsPoint(visibleRect, v2p);
        }
        else
        {
            const auto modelViewProjection = getViewProjection() * boxTransform;
            const auto frustum = getFrustum(modelViewProjection);
            return isBoxInside(frustum, box);
        }
    }

    void Camera::setViewport(const math::Rect<float>& newViewport)
    {
        viewport = newViewport;
        projectionDirty = viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::setScaleMode(ScaleMode newScaleMode)
    {
        scaleMode = newScaleMode;
        projectionDirty = viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::setTargetContentSize(const math::Size<float, 2>& newTargetContentSize)
    {
        targetContentSize = newTargetContentSize;
        projectionDirty = viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::setRenderTarget(graphics::RenderTarget* newRenderTarget)
    {
        renderTarget = newRenderTarget;
        projectionDirty = viewProjectionDirty = inverseViewProjectionDirty = true;
    }

    void Camera::setDepthTest(bool newDepthTest)
    {
        depthTest = newDepthTest;

        if (depthTest)
            depthStencilState = std::make_unique<graphics::DepthStencilState>(engine->getGraphics(),
                                                                              true, true,
                                                                              graphics::CompareFunction::lessEqual,
                                                                              false,
                                                                              0xFFFFFFFF,
                                                                              0xFFFFFFFF,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::CompareFunction::always,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::StencilOperation::keep,
                                                                              graphics::CompareFunction::always);
        else
            depthStencilState.reset();
    }
}
