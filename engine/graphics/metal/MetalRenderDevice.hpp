// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_METALRENDERDEVICE_HPP
#define OUZEL_GRAPHICS_METALRENDERDEVICE_HPP

#include "../../core/Setup.h"

#if OUZEL_COMPILE_METAL

#include <array>
#include <map>
#include <memory>
#include <vector>

#ifdef __OBJC__
#  import <CoreVideo/CoreVideo.h>
#  import <QuartzCore/QuartzCore.h>
#  import <Metal/Metal.h>
using CAMetalLayerPtr = CAMetalLayer*;
using MTLDevicePtr = id<MTLDevice>;
using MTLBufferPtr = id<MTLBuffer>;
using MTLRenderPassDescriptorPtr = MTLRenderPassDescriptor*;
using MTLSamplerStatePtr = id<MTLSamplerState>;
using MTLCommandQueuePtr = id<MTLCommandQueue>;
using MTLRenderPipelineStatePtr = id<MTLRenderPipelineState>;
using MTLTexturePtr = id<MTLTexture>;
using MTLDepthStencilStatePtr = id<MTLDepthStencilState>;
#else
#  include <objc/objc.h>
#  include <objc/NSObjCRuntime.h>
using CAMetalLayerPtr = id;
using MTLDevicePtr = id;
using MTLBufferPtr = id;
using MTLRenderPassDescriptorPtr = id;
using MTLSamplerStatePtr = id;
using MTLCommandBufferPtr = id;
using MTLCommandQueuePtr = id;
using MTLRenderCommandEncoderPtr = id;
using MTLRenderPipelineStatePtr = id;
using MTLTexturePtr = id;
using MTLDepthStencilStatePtr = id;
using MTLPixelFormat = NSUInteger;
#endif

#include "../RenderDevice.hpp"
#include "MetalShader.hpp"
#include "MetalTexture.hpp"
#include "../../platform/dispatch/Semaphore.hpp"
#include "../../platform/objc/Pointer.hpp"

namespace ouzel::graphics::metal
{
    class BlendState;
    class Shader;

    class RenderDevice: public graphics::RenderDevice
    {
        friend Graphics;
    public:
        static constexpr std::size_t bufferSize = 1024U * 1024U; // size of shader constant buffer
        static constexpr std::size_t bufferCount = 3U; // allow encoding up to 3 command buffers simultaneously

        static bool available() noexcept;

        auto& getDevice() const noexcept { return device; }

        MTLSamplerStatePtr getSamplerState(const SamplerStateDescriptor& descriptor);

        template <class T>
        auto getResource(std::size_t id) const
        {
            return id ? static_cast<T*>(resources[id - 1].get()) : nullptr;
        }

    protected:
        RenderDevice(const Settings& settings,
                     core::Window& newWindow);

        void process() override;
        void generateScreenshot(const std::string& filename) override;

        class PipelineStateDesc final
        {
        public:
            BlendState* blendState = nullptr;
            Shader* shader = nullptr;
            NSUInteger sampleCount = 0;
            std::vector<MTLPixelFormat> colorFormats;
            MTLPixelFormat depthFormat;
            MTLPixelFormat stencilFormat;

            bool operator<(const PipelineStateDesc& other) const noexcept
            {
                return std::tie(blendState, shader, sampleCount, colorFormats, depthFormat) <
                    std::tie(other.blendState, other.shader, other.sampleCount, colorFormats, other.depthFormat);
            }
        };

        MTLRenderPipelineStatePtr getPipelineState(const PipelineStateDesc& desc);

        platform::objc::Pointer<MTLDevicePtr> device;
        platform::objc::Pointer<MTLCommandQueuePtr> metalCommandQueue;
        CAMetalLayerPtr metalLayer = nil;
        platform::objc::Pointer<MTLTexturePtr> currentMetalTexture;

        struct ShaderConstantBuffer final
        {
            std::vector<platform::objc::Pointer<MTLBufferPtr>> buffers;
            std::uint32_t index = 0;
            std::uint32_t offset = 0;
        };

        std::uint32_t shaderConstantBufferIndex = 0;
        std::array<ShaderConstantBuffer, bufferCount> shaderConstantBuffers;

        platform::objc::Pointer<MTLRenderPassDescriptorPtr> renderPassDescriptor;
        platform::objc::Pointer<MTLDepthStencilStatePtr> defaultDepthStencilState;

        platform::objc::Pointer<MTLTexturePtr> msaaTexture;
        platform::objc::Pointer<MTLTexturePtr> depthTexture;
        std::map<SamplerStateDescriptor, platform::objc::Pointer<MTLSamplerStatePtr>> samplerStates;

        MTLPixelFormat colorFormat;
        MTLPixelFormat depthFormat;
        MTLPixelFormat stencilFormat;

        platform::dispatch::Semaphore inflightSemaphore{bufferCount};

        std::map<PipelineStateDesc, platform::objc::Pointer<MTLRenderPipelineStatePtr>> pipelineStates;

        std::vector<std::unique_ptr<RenderResource>> resources;
    };
}

#endif

#endif // OUZEL_GRAPHICS_METALRENDERDEVICE_HPP
