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
    RenderDevice::RenderDevice(const Settings& settings,
                               core::Window& initWindow,
                               const std::function<void(const Event&)>& initCallback):
        metal::RenderDevice{settings, initWindow, initCallback},
        displayLink{std::bind(&RenderDevice::renderCallback, this)}
    {
        const auto windowIOS = static_cast<core::ios::NativeWindow*>(window.getNativeWindow());
        MetalView* view = (MetalView*)windowIOS->getNativeView();

        metalLayer = (CAMetalLayer*)view.layer;
        metalLayer.device = device.get();
        const CGSize drawableSize = CGSizeMake(window.getResolution().v[0],
                                               window.getResolution().v[1]);
        metalLayer.drawableSize = drawableSize;

        colorFormat = metalLayer.pixelFormat;
    }

    RenderDevice::~RenderDevice()
    {
        running = false;
        runLoop.stop();
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));
    }

    void RenderDevice::start()
    {
        renderThread = thread::Thread{&RenderDevice::renderMain, this};
        std::unique_lock lock{runLoopMutex};
        while (!running) runLoopCondition.wait(lock);
    }

    void RenderDevice::renderMain()
    {
        thread::setCurrentThreadName("Render");

        if (verticalSync)
        {
            runLoop = platform::foundation::RunLoop{};
            displayLink.addToRunLoop(runLoop);

            runLoop.performFunction([this](){
                std::unique_lock lock{runLoopMutex};
                running = true;
                lock.unlock();
                runLoopCondition.notify_all();
            });

            runLoop.run();
        }
        else
        {
            std::unique_lock lock{runLoopMutex};
            running = true;
            lock.unlock();

            runLoopCondition.notify_all();

            while (running) renderCallback();
        }
    }

    void RenderDevice::renderCallback()
    {
        try
        {
            platform::foundation::AutoreleasePool autoreleasePool;
            process();
        }
        catch (const std::exception& e)
        {
            logger.log(Log::Level::error) << e.what();
        }
    }
}

#endif
