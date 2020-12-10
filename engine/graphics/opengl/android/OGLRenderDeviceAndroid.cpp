// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../../core/Setup.h"

#if defined(__ANDROID__) && OUZEL_COMPILE_OPENGL

#include <algorithm>
#include "OGLRenderDeviceAndroid.hpp"
#include "../EGLErrorCategory.hpp"
#include "../../../core/Engine.hpp"
#include "../../../core/Window.hpp"
#include "../../../core/android/NativeWindowAndroid.hpp"
#include "../../../utils/Log.hpp"
#include "../../../utils/Utils.hpp"

namespace ouzel::graphics::opengl::android
{
    namespace
    {
        const egl::ErrorCategory eglErrorCategory{};
    }

    RenderDevice::RenderDevice(const Settings& settings,
                               core::Window& initWindow,
                               const std::function<void(const Event&)>& initCallback):
        opengl::RenderDevice(settings, initWindow, initCallback)
    {
        embedded = true;

        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

        if (display == EGL_NO_DISPLAY)
            throw std::runtime_error("Failed to get display");

        if (!eglInitialize(display, nullptr, nullptr))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to initialize EGL");

        const auto eglVersionPtr = eglQueryString(display, EGL_VERSION);
        if (!eglVersionPtr)
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get EGL version");
        logger.log(Log::Level::all) << "EGL version: " << eglVersionPtr;

        const auto eglExtensionsPtr = eglQueryString(display, EGL_EXTENSIONS);
        if (!eglExtensionsPtr)
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get EGL extensions");
        const auto eglExtensions = explodeString(eglExtensionsPtr, ' ');
        logger.log(Log::Level::all) << "Supported EGL extensions: " << eglExtensions;

        auto windowAndroid = static_cast<core::android::NativeWindow*>(window.getNativeWindow());

        if (std::find(eglExtensions.begin(), eglExtensions.end(), "EGL_KHR_create_context") != eglExtensions.end())
        {
            const EGLint attributeList[] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, settings.depth ? 24 : 0,
                EGL_STENCIL_SIZE, settings.stencil ? 8 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (settings.sampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(settings.sampleCount),
                EGL_NONE
            };
            
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, nullptr, 0, &numConfig))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to choose EGL config");

            if (numConfig == 0)
                throw std::runtime_error("No EGL config found");

            std::vector<EGLConfig> configs(numConfig);
            if (!eglChooseConfig(display, attributeList, configs.data(), static_cast<EGLint>(configs.size()), &numConfig))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to choose EGL config");

            if (!eglBindAPI(EGL_OPENGL_ES_API))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to bind OpenGL ES API");

            EGLint format;
            if (!eglGetConfigAttrib(display, configs[0], EGL_NATIVE_VISUAL_ID, &format))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get config attribute");

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, configs[0], windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to create EGL window surface");

            const EGLint contextAttributes[] = {
                EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
                EGL_CONTEXT_MINOR_VERSION_KHR, 0,
                EGL_CONTEXT_OPENGL_DEBUG, settings.debugRenderer ? EGL_TRUE : EGL_FALSE,
                EGL_NONE
            };

            context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, contextAttributes);

            if (context != EGL_NO_CONTEXT)
            {
                apiVersion = ApiVersion(3, 0);
                logger.log(Log::Level::info) << "EGL OpenGL ES " << 3 << " context created";
            }
            else // TODO: use RAII for surface
                eglDestroySurface(display, surface);
        }

        if (context == EGL_NO_CONTEXT)
        {
            const EGLint attributeList[] = {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, settings.depth ? 24 : 0,
                EGL_STENCIL_SIZE, settings.stencil ? 8 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (settings.sampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(settings.sampleCount),
                EGL_NONE
            };
            
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, nullptr, 0, &numConfig))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to choose EGL config");

            if (numConfig == 0)
                throw std::runtime_error("No EGL config found");

            std::vector<EGLConfig> configs(numConfig);
            if (!eglChooseConfig(display, attributeList, configs.data(), static_cast<EGLint>(configs.size()), &numConfig))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to choose EGL config");

            if (!eglBindAPI(EGL_OPENGL_ES_API))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to bind OpenGL ES API");

            EGLint format;
            if (!eglGetConfigAttrib(display, configs[0], EGL_NATIVE_VISUAL_ID, &format))
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get config attribute");

            auto windowAndroid = static_cast<core::android::NativeWindow*>(window.getNativeWindow());

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, configs[0], windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
                throw std::system_error(eglGetError(), eglErrorCategory, "Failed to create EGL window surface");

            const EGLint contextAttributes[] = {
                EGL_CONTEXT_CLIENT_VERSION, 2,
                EGL_CONTEXT_OPENGL_DEBUG, settings.debugRenderer ? EGL_TRUE : EGL_FALSE,
                EGL_NONE
            };

            context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, contextAttributes);

            if (context != EGL_NO_CONTEXT)
            {
                apiVersion = ApiVersion(2, 0);
                logger.log(Log::Level::info) << "EGL OpenGL ES " << 2 << " context created";
            }
        }

        if (context == EGL_NO_CONTEXT)
            throw std::runtime_error("Failed to create EGL context");

        if (!eglMakeCurrent(display, surface, surface, context))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to set current EGL context");

        if (!eglSwapInterval(display, settings.verticalSync ? 1 : 0))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to set EGL frame interval");

        EGLint surfaceWidth;
        EGLint surfaceHeight;

        if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
            !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get query window size");

        init(surfaceWidth, surfaceHeight);

        if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to unset EGL context");

        running = true;
        renderThread = thread::Thread(&RenderDevice::renderMain, this);
    }

    RenderDevice::~RenderDevice()
    {
        running = false;
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));

        if (renderThread.isJoinable()) renderThread.join();

        if (context != EGL_NO_CONTEXT)
        {
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            eglDestroyContext(display, context);
        }

        if (surface != EGL_NO_SURFACE)
            eglDestroySurface(display, surface);

        if (display != EGL_NO_DISPLAY)
            eglTerminate(display);
    }

    void RenderDevice::reload()
    {
        running = false;
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));

        if (renderThread.isJoinable()) renderThread.join();

        const EGLint attributeList[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, depth ? 24 : 0,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_SAMPLE_BUFFERS, (sampleCount > 1) ? 1 : 0,
            EGL_SAMPLES, static_cast<int>(sampleCount),
            EGL_NONE
        };
        EGLConfig config;
        EGLint numConfig;
        if (!eglChooseConfig(display, attributeList, &config, 1, &numConfig))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to choose EGL config");

        if (!eglBindAPI(EGL_OPENGL_ES_API))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to bind OpenGL ES API");

        EGLint format;
        if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get config attribute");

        auto windowAndroid = static_cast<core::android::NativeWindow*>(window.getNativeWindow());

        ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

        surface = eglCreateWindowSurface(display, config, windowAndroid->getNativeWindow(), nullptr);
        if (surface == EGL_NO_SURFACE)
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to create EGL window surface");

        for (EGLint version = 3; version >= 2; --version)
        {
            const EGLint contextAttributes[] = {
                EGL_CONTEXT_CLIENT_VERSION, version,
                EGL_CONTEXT_OPENGL_DEBUG, debugRenderer ? EGL_TRUE : EGL_FALSE,
                EGL_NONE
            };

            context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);

            if (context != EGL_NO_CONTEXT)
            {
                apiVersion = ApiVersion(version, 0);
                logger.log(Log::Level::info) << "EGL OpenGL ES " << version << " context created";
                break;
            }
        }

        if (context == EGL_NO_CONTEXT)
            throw std::runtime_error("Failed to create EGL context");

        if (!eglMakeCurrent(display, surface, surface, context))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to set current EGL context");

        if (!eglSwapInterval(display, verticalSync ? 1 : 0))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to set EGL frame interval");

        EGLint surfaceWidth;
        EGLint surfaceHeight;

        if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
            !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to get query window size");

        frameBufferWidth = surfaceWidth;
        frameBufferHeight = surfaceHeight;

        stateCache = StateCache();

        glDisableProc(GL_DITHER);
        glDepthFuncProc(GL_LEQUAL);

        if (const auto error = glGetErrorProc(); error != GL_NO_ERROR)
            throw std::system_error(makeErrorCode(error), "Failed to set depth function");

        if (glGenVertexArraysProc) glGenVertexArraysProc(1, &vertexArrayId);

        for (const auto& resource : resources)
            if (resource) resource->invalidate();

        for (const auto& resource : resources)
            if (resource) resource->restore();

        if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to unset EGL context");

        running = true;
        renderThread = thread::Thread(&RenderDevice::renderMain, this);
    }

    void RenderDevice::destroy()
    {
        running = false;
        CommandBuffer commandBuffer;
        commandBuffer.pushCommand(std::make_unique<PresentCommand>());
        submitCommandBuffer(std::move(commandBuffer));

        if (renderThread.isJoinable()) renderThread.join();

        if (context)
        {
            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                logger.log(Log::Level::error) << "Failed to unset EGL context";

            if (!eglDestroyContext(display, context))
                logger.log(Log::Level::error) << "Failed to destroy EGL context";

            context = nullptr;
        }

        if (surface)
        {
            if (!eglDestroySurface(display, surface))
                logger.log(Log::Level::error) << "Failed to destroy EGL surface";

            surface = nullptr;
        }
    }

    void RenderDevice::present()
    {
        if (eglSwapBuffers(display, surface) != EGL_TRUE)
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to swap buffers");
    }

    void RenderDevice::renderMain()
    {
        thread::setCurrentThreadName("Render");

        if (!eglMakeCurrent(display, surface, surface, context))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to set current EGL context");

        while (running)
        {
            try
            {
                process();
            }
            catch (const std::exception& e)
            {
                logger.log(Log::Level::error) << e.what();
            }
        }

        if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
            throw std::system_error(eglGetError(), eglErrorCategory, "Failed to unset EGL context");
    }
}

#endif
