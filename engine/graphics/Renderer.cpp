// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif
#include <stdexcept>
#include "../core/Setup.h"
#include "Renderer.hpp"
#include "Commands.hpp"
#include "RenderDevice.hpp"
#include "../events/EventHandler.hpp"
#include "../events/EventDispatcher.hpp"
#include "../core/Engine.hpp"
#include "../core/Window.hpp"
#include "../utils/Log.hpp"

#if TARGET_OS_IOS
#  include "metal/ios/MetalRenderDeviceIOS.hpp"
#  include "opengl/ios/OGLRenderDeviceIOS.hpp"
#elif TARGET_OS_TV
#  include "metal/tvos/MetalRenderDeviceTVOS.hpp"
#  include "opengl/tvos/OGLRenderDeviceTVOS.hpp"
#elif TARGET_OS_MAC
#  include "metal/macos/MetalRenderDeviceMacOS.hpp"
#  include "opengl/macos/OGLRenderDeviceMacOS.hpp"
#elif defined(__ANDROID__)
#  include "opengl/android/OGLRenderDeviceAndroid.hpp"
#elif defined(__linux__)
#  include "opengl/linux/OGLRenderDeviceLinux.hpp"
#elif defined(_WIN32)
#  include "opengl/windows/OGLRenderDeviceWin.hpp"
#elif defined(__EMSCRIPTEN__)
#  include "opengl/emscripten/OGLRenderDeviceEm.hpp"
#endif

#include "empty/EmptyRenderDevice.hpp"
#include "opengl/OGLRenderDevice.hpp"
#include "direct3d11/D3D11RenderDevice.hpp"
#include "metal/MetalRenderDevice.hpp"

namespace ouzel::graphics
{
    Driver Renderer::getDriver(const std::string& driver)
    {
        if (driver.empty() || driver == "default")
        {
            auto availableDrivers = getAvailableRenderDrivers();

            if (availableDrivers.find(Driver::metal) != availableDrivers.end())
                return Driver::metal;
            else if (availableDrivers.find(Driver::direct3D11) != availableDrivers.end())
                return Driver::direct3D11;
            else if (availableDrivers.find(Driver::openGL) != availableDrivers.end())
                return Driver::openGL;
            else
                return Driver::empty;
        }
        else if (driver == "empty")
            return Driver::empty;
        else if (driver == "opengl")
            return Driver::openGL;
        else if (driver == "direct3d11")
            return Driver::direct3D11;
        else if (driver == "metal")
            return Driver::metal;
        else
            throw std::runtime_error("Invalid graphics driver");
    }

    std::set<Driver> Renderer::getAvailableRenderDrivers()
    {
        static std::set<Driver> availableDrivers;

        if (availableDrivers.empty())
        {
            availableDrivers.insert(Driver::empty);

#if OUZEL_COMPILE_OPENGL
            availableDrivers.insert(Driver::openGL);
#endif
#if OUZEL_COMPILE_DIRECT3D11
            availableDrivers.insert(Driver::direct3D11);
#endif
#if OUZEL_COMPILE_METAL
            if (metal::RenderDevice::available())
                availableDrivers.insert(Driver::metal);
#endif
        }

        return availableDrivers;
    }

    Renderer::Renderer(Driver driver,
                       Window* newWindow,
                       const Size2U& newSize,
                       std::uint32_t newSampleCount,
                       SamplerFilter newTextureFilter,
                       std::uint32_t newMaxAnisotropy,
                       bool newSrgb,
                       bool newVerticalSync,
                       bool newDepth,
                       bool newStencil,
                       bool newDebugRenderer):
        textureFilter(newTextureFilter),
        maxAnisotropy(newMaxAnisotropy)
    {
        switch (driver)
        {
#if OUZEL_COMPILE_OPENGL
            case Driver::openGL:
                engine->log(Log::Level::info) << "Using OpenGL render driver";
#  if TARGET_OS_IOS
                device = std::make_unique<opengl::RenderDeviceIOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif TARGET_OS_TV
                device = std::make_unique<opengl::RenderDeviceTVOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif TARGET_OS_MAC
                device = std::make_unique<opengl::RenderDeviceMacOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif defined(__ANDROID__)
                device = std::make_unique<opengl::RenderDeviceAndroid>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif defined(__linux__)
                device = std::make_unique<opengl::RenderDeviceLinux>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif defined(_WIN32)
                device = std::make_unique<opengl::RenderDeviceWin>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif defined(__EMSCRIPTEN__)
                device = std::make_unique<opengl::RenderDeviceEm>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  else
                device = std::make_unique<opengl::RenderDevice>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  endif
                break;
#endif
#if OUZEL_COMPILE_DIRECT3D11
            case Driver::direct3D11:
                engine->log(Log::Level::info) << "Using Direct3D 11 render driver";
                device = std::make_unique<d3d11::RenderDevice>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
                break;
#endif
#if OUZEL_COMPILE_METAL
            case Driver::metal:
                engine->log(Log::Level::info) << "Using Metal render driver";
#  if TARGET_OS_IOS
                device = std::make_unique<metal::RenderDeviceIOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif TARGET_OS_TV
                device = std::make_unique<metal::RenderDeviceTVOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  elif TARGET_OS_MAC
                device = std::make_unique<metal::RenderDeviceMacOS>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
#  endif
                break;
#endif
            default:
                engine->log(Log::Level::info) << "Not using render driver";
                device = std::make_unique<empty::RenderDevice>(std::bind(&Renderer::handleEvent, this, std::placeholders::_1));
                break;
        }

        size = newSize;

        device->init(newWindow,
                     newSize,
                     newSampleCount,
                     newSrgb,
                     newVerticalSync,
                     newDepth,
                     newStencil,
                     newDebugRenderer);
    }

    void Renderer::handleEvent(const RenderDevice::Event& event)
    {
        if (event.type == RenderDevice::Event::Type::frame)
        {
            std::unique_lock lock(frameMutex);
            newFrame = true;
            refillQueue = true;
            lock.unlock();
            frameCondition.notify_all();
        }
    }

    void Renderer::setSize(const Size2U& newSize)
    {
        size = newSize;

        addCommand(std::make_unique<ResizeCommand>(newSize));
    }

    void Renderer::saveScreenshot(const std::string& filename)
    {
        device->executeOnRenderThread(std::bind(&RenderDevice::generateScreenshot, device.get(), filename));
    }

    void Renderer::setRenderTarget(std::uintptr_t renderTarget)
    {
        addCommand(std::make_unique<SetRenderTargetCommand>(renderTarget));
    }

    void Renderer::clearRenderTarget(bool clearColorBuffer,
                                     bool clearDepthBuffer,
                                     bool clearStencilBuffer,
                                     Color clearColor,
                                     float clearDepth,
                                     std::uint32_t clearStencil)
    {
        addCommand(std::make_unique<ClearRenderTargetCommand>(clearColorBuffer,
                                                              clearDepthBuffer,
                                                              clearStencilBuffer,
                                                              clearColor,
                                                              clearDepth,
                                                              clearStencil));
    }

    void Renderer::setScissorTest(bool enabled, const RectF& rectangle)
    {
        addCommand(std::make_unique<SetScissorTestCommand>(enabled, rectangle));
    }

    void Renderer::setViewport(const RectF& viewport)
    {
        addCommand(std::make_unique<SetViewportCommand>(viewport));
    }

    void Renderer::setDepthStencilState(std::uintptr_t depthStencilState,
                                        std::uint32_t stencilReferenceValue)
    {
        addCommand(std::make_unique<SetDepthStencilStateCommand>(depthStencilState,
                                                                 stencilReferenceValue));
    }

    void Renderer::setPipelineState(std::uintptr_t blendState,
                                    std::uintptr_t shader,
                                    CullMode cullMode,
                                    FillMode fillMode)
    {
        addCommand(std::make_unique<SetPipelineStateCommand>(blendState,
                                                             shader,
                                                             cullMode,
                                                             fillMode));
    }

    void Renderer::draw(std::uintptr_t indexBuffer,
                        std::uint32_t indexCount,
                        std::uint32_t indexSize,
                        std::uintptr_t vertexBuffer,
                        DrawMode drawMode,
                        std::uint32_t startIndex)
    {
        if (!indexBuffer || !vertexBuffer)
            throw std::runtime_error("Invalid mesh buffer passed to render queue");

        addCommand(std::make_unique<DrawCommand>(indexBuffer,
                                                 indexCount,
                                                 indexSize,
                                                 vertexBuffer,
                                                 drawMode,
                                                 startIndex));
    }

    void Renderer::pushDebugMarker(const std::string& name)
    {
        addCommand(std::make_unique<PushDebugMarkerCommand>(name));
    }

    void Renderer::popDebugMarker()
    {
        addCommand(std::make_unique<PopDebugMarkerCommand>());
    }

    void Renderer::setShaderConstants(const std::vector<std::vector<float>>& fragmentShaderConstants,
                                      const std::vector<std::vector<float>>& vertexShaderConstants)
    {
        addCommand(std::make_unique<SetShaderConstantsCommand>(fragmentShaderConstants,
                                                               vertexShaderConstants));
    }

    void Renderer::setTextures(const std::vector<std::uintptr_t>& textures)
    {
        addCommand(std::make_unique<SetTexturesCommand>(textures));
    }

    void Renderer::present()
    {
        refillQueue = false;
        addCommand(std::make_unique<PresentCommand>());
        device->submitCommandBuffer(std::move(commandBuffer));
        commandBuffer = CommandBuffer();
    }

    void Renderer::waitForNextFrame()
    {
        std::unique_lock lock(frameMutex);
        while (!newFrame) frameCondition.wait(lock);
        newFrame = false;
    }
}
