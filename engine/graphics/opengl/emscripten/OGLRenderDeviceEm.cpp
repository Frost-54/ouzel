// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../../core/Setup.h"

#if defined(__EMSCRIPTEN__) && OUZEL_COMPILE_OPENGL

#include "OGLRenderDeviceEm.hpp"
#include "../../../core/Window.hpp"
#include "../../../utils/Utils.hpp"

namespace ouzel::graphics::opengl::emscripten
{
    RenderDevice::RenderDevice(const Settings& settings,
                               core::Window& initWindow,
                               const std::function<void(const Event&)>& initCallback):
        opengl::RenderDevice(settings, initWindow, initCallback)
    {
        embedded = true;

        apiVersion = ApiVersion(2, 0);

        EmscriptenWebGLContextAttributes attrs;
        emscripten_webgl_init_context_attributes(&attrs);

        attrs.alpha = true;
        attrs.depth = settings.depth;
        attrs.stencil = settings.stencil;
        attrs.antialias = settings.sampleCount > 0;

        webGLContext = emscripten_webgl_create_context(0, &attrs);

        if (!webGLContext)
            throw std::runtime_error("Failed to create WebGL context");

        if (const auto result = emscripten_webgl_make_context_current(webGLContext); result != EMSCRIPTEN_RESULT_SUCCESS)
            throw std::runtime_error("Failed to make WebGL context current");

        emscripten_set_main_loop_timing(settings.verticalSync ? EM_TIMING_RAF : EM_TIMING_SETTIMEOUT, 1);

        init(static_cast<GLsizei>(window.getResolution().v[0]),
             static_cast<GLsizei>(window.getResolution().v[1]));
    }

    RenderDevice::~RenderDevice()
    {
        if (webGLContext)
            emscripten_webgl_destroy_context(webGLContext);
    }
}

#endif
