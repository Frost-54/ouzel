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

    RenderDevice::RenderDevice(const std::function<void(const Event&)>& initCallback):
        metal::RenderDevice(initCallback),
        displayLink(tvos::renderCallback, this)
    {
    }

    RenderDevice::~RenderDevice()
    {
        displayLink.stop();
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));
    }

    void RenderDevice::init(core::Window& newWindow,
                            const Size2U& newSize,
                            const Settings& settings)
    {
        metal::RenderDevice::init(newWindow, newSize, settings);

        auto windowTVOS = static_cast<core::tvos::NativeWindow*>(window->getNativeWindow());
        MetalView* view = (MetalView*)windowTVOS->getNativeView();

        metalLayer = (CAMetalLayer*)view.layer;
        metalLayer.device = device.get();
        const CGSize drawableSize = CGSizeMake(newSize.v[0], newSize.v[1]);
        metalLayer.drawableSize = drawableSize;

        colorFormat = metalLayer.pixelFormat;

        displayLink.start(verticalSync);
    }

    void RenderDevice::renderCallback()
    {
        process();
    }
}

#endif
