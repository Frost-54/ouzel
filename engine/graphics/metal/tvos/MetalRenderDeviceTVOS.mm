// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../../core/Setup.h"

#if TARGET_OS_TV && OUZEL_COMPILE_METAL

#include "MetalRenderDeviceTVOS.hpp"
#include "MetalView.h"
#include "../../../core/Engine.hpp"
#include "../../../core/Window.hpp"
#include "../../../core/tvos/NativeWindowTVOS.hpp"
#include "../../../utils/Log.hpp"

namespace ouzel::graphics::metal::tvos
{
    namespace
    {
        void renderCallback(void* userInfo)
        {
            try
            {
                auto renderDevice = static_cast<RenderDevice*>(userInfo);
                renderDevice->renderCallback();
            }
            catch (const std::exception& e)
            {
                logger.log(Log::Level::error) << e.what();
            }
        }
    }

    RenderDevice::RenderDevice(const Settings& settings,
                               core::Window& initWindow,
                               const std::function<void(const Event&)>& initCallback):
        metal::RenderDevice(settings, initWindow, initCallback),
        displayLink(tvos::renderCallback, this)
    {
        auto windowTVOS = static_cast<core::tvos::NativeWindow*>(window.getNativeWindow());
        MetalView* view = (MetalView*)windowTVOS->getNativeView();

        metalLayer = (CAMetalLayer*)view.layer;
        metalLayer.device = device.get();
        const CGSize drawableSize = CGSizeMake(window.getResolution().v[0],
                                               window.getResolution().v[1]);
        metalLayer.drawableSize = drawableSize;

        colorFormat = metalLayer.pixelFormat;

        displayLink.start(verticalSync);
    }

    RenderDevice::~RenderDevice()
    {
        displayLink.stop();
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));
    }

    void RenderDevice::renderCallback()
    {
        process();
    }
}

#endif
