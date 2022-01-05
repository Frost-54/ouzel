// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_OGLRENDERDEVICEIOS_HPP
#define OUZEL_GRAPHICS_OGLRENDERDEVICEIOS_HPP

#include "../../../core/Setup.h"

#ifdef __APPLE__
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_IOS && OUZEL_COMPILE_OPENGL

#include "../OGLRenderDevice.hpp"
#include "../../../thread/Thread.hpp"
#include "../../../platform/foundation/RunLoop.hpp"
#include "../../../platform/quartzcore/DisplayLink.hpp"

#ifdef __OBJC__
#  import <UIKit/UIKit.h>
using EAGLSharegroupPtr = EAGLSharegroup*;
using EAGLContextPtr = EAGLContext*;
using CAEAGLLayerPtr = CAEAGLLayer*;
#else
#  include <objc/objc.h>
using EAGLSharegroupPtr = id;
using EAGLContextPtr = id;
using CAEAGLLayerPtr = id;
#endif

namespace ouzel::graphics::opengl::ios
{
    class RenderDevice final: public opengl::RenderDevice
    {
        friend Graphics;
    public:
        RenderDevice(const Settings& settings,
                     core::Window& initWindow);
        ~RenderDevice() override;

        void start() final;
        void renderCallback();

    private:
        void resizeFrameBuffer() final;
        void present() final;

        void createFrameBuffer();

        void renderMain();

        EAGLSharegroupPtr shareGroup = nil;
        EAGLContextPtr context = nil;
        CAEAGLLayerPtr eaglLayer = nil;

        GLuint msaaFrameBufferId = 0;
        GLuint msaaColorRenderBufferId = 0;

        GLuint resolveFrameBufferId = 0;
        GLuint resolveColorRenderBufferId = 0;

        GLuint depthRenderBufferId = 0;

        thread::Thread renderThread;
        std::atomic_bool running{false};
        std::mutex runLoopMutex;
        std::condition_variable runLoopCondition;
        bool started = false;
        platform::foundation::RunLoop runLoop;
        platform::quartzcore::DisplayLink displayLink;
    };
}
#endif

#endif // OUZEL_GRAPHICS_OGLRENDERDEVICEIOS_HPP
