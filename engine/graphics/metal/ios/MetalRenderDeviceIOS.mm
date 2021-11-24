// Ouzel by Elviss Strazdins

#include "../../../core/Setup.h"

#ifdef __APPLE__
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_IOS && OUZEL_COMPILE_METAL

#include "MetalRenderDeviceIOS.hpp"
#include "MetalView.h"
#include "../../../core/Engine.hpp"
#include "../../../core/Window.hpp"
#include "../../../core/ios/NativeWindowIOS.hpp"
#include "../../../platform/foundation/AutoreleasePool.hpp"
#include "../../../utils/Log.hpp"

namespace ouzel::graphics::metal::ios
{
    namespace
    {
        void renderCallback(void* userInfo)
        {
            platform::foundation::AutoreleasePool autoreleasePool;

            try
            {
                const auto renderDevice = static_cast<RenderDevice*>(userInfo);
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
        metal::RenderDevice{settings, initWindow, initCallback},
        displayLink{ios::renderCallback, this}
    {
        const auto windowIOS = static_cast<core::ios::NativeWindow*>(window.getNativeWindow());
        MetalView* view = (MetalView*)windowIOS->getNativeView();

        metalLayer = (CAMetalLayer*)view.layer;
        metalLayer.device = device.get();
        const CGSize drawableSize = CGSizeMake(window.getResolution().v[0],
                                               window.getResolution().v[1]);
        metalLayer.drawableSize = drawableSize;

        colorFormat = metalLayer.pixelFormat;

        running = true;
        renderThread = thread::Thread{&RenderDevice::renderMain, this};
    }

    RenderDevice::~RenderDevice()
    {
        running = false;
        displayLink.stop();
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));
    }

    void RenderDevice::renderMain()
    {
        thread::setCurrentThreadName("Render");

        if (verticalSync)
            displayLink.start();
        else while (running)
            process();
    }

    void RenderDevice::renderCallback()
    {
        process();
    }
}

#endif
