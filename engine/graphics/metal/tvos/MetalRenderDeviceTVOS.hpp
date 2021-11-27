// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_METALRENDERDEVICETVOS_HPP
#define OUZEL_GRAPHICS_METALRENDERDEVICETVOS_HPP

#include "../../../core/Setup.h"

#ifdef __APPLE__
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_TV && OUZEL_COMPILE_METAL

#include "../MetalRenderDevice.hpp"
#include "../../../thread/Thread.hpp"
#include "../../../platform/foundation/RunLoop.hpp"
#include "../../../platform/quartzcore/DisplayLink.hpp"

namespace ouzel::graphics::metal::tvos
{
    class RenderDevice final: public metal::RenderDevice
    {
        friend Graphics;
    public:
        RenderDevice(const Settings& settings,
                     core::Window& initWindow,
                     const std::function<void(const Event&)>& initCallback);
        ~RenderDevice() override;

        void renderCallback();

    private:
        void renderMain();

        thread::Thread renderThread;
        std::atomic_bool running{false};
        std::mutex runLoopMutex;
        std::condition_variable runLoopCondition;
        platform::foundation::RunLoop runLoop;
        platform::quartzcore::DisplayLink displayLink;
    };
}

#endif

#endif // OUZEL_GRAPHICS_METALRENDERDEVICETVOS_HPP
