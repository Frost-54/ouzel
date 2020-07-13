// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_OGLRENDERDEVICEMACOS_HPP
#define OUZEL_GRAPHICS_OGLRENDERDEVICEMACOS_HPP

#include "../../../core/Setup.h"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_MAC && !TARGET_OS_IOS && !TARGET_OS_TV && OUZEL_COMPILE_OPENGL

#if defined(__OBJC__)
#  import <CoreVideo/CoreVideo.h>
#  import <AppKit/NSOpenGL.h>
typedef NSOpenGLContext* NSOpenGLContextPtr;
typedef NSOpenGLPixelFormat* NSOpenGLPixelFormatPtr;
#else
#  include <objc/objc.h>
typedef void* CVDisplayLinkRef;
typedef id NSOpenGLContextPtr;
typedef id NSOpenGLPixelFormatPtr;
#endif

#include "../OGLRenderDevice.hpp"
#include "../../../events/EventHandler.hpp"

namespace ouzel::graphics::opengl::macos
{
    class RenderDevice final: public opengl::RenderDevice
    {
        friend Graphics;
    public:
        explicit RenderDevice(const std::function<void(const Event&)>& initCallback);
        ~RenderDevice() override;

        std::vector<Size2U> getSupportedResolutions() const final;

        auto getOpenGLContext() const noexcept { return openGLContext; }

        void renderCallback();

    private:
        void init(core::Window* newWindow,
                  const Size2U& newSize,
                  const Settings& settings) final;

        void resizeFrameBuffer() final;
        void present() final;

        bool handleWindow(const WindowEvent& event);

        NSOpenGLContextPtr openGLContext = nil;
        NSOpenGLPixelFormatPtr pixelFormat = nil;

        CVDisplayLinkRef displayLink = nullptr;
        EventHandler eventHandler;

        std::atomic_bool running{false};
    };
}
#endif

#endif // OUZEL_GRAPHICS_OGLRENDERDEVICEMACOS_HPP
