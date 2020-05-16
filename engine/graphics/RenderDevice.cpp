// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <algorithm>
#include "RenderDevice.hpp"

namespace ouzel
{
    namespace graphics
    {
        const std::array<Vertex::Attribute, 5> RenderDevice::VERTEX_ATTRIBUTES{
            Vertex::Attribute{Vertex::Attribute::Usage::position, DataType::float32Vector3},
            Vertex::Attribute{Vertex::Attribute::Usage::color, DataType::unsignedByteVector4Norm},
            Vertex::Attribute{Vertex::Attribute::Usage::textureCoordinates0, DataType::float32Vector2},
            Vertex::Attribute{Vertex::Attribute::Usage::textureCoordinates1, DataType::float32Vector2},
            Vertex::Attribute{Vertex::Attribute::Usage::normal, DataType::float32Vector3}
        };

        RenderDevice::RenderDevice(Driver initDriver,
                                   const std::function<void(const Event&)>& initCallback):
            driver(initDriver),
            callback(initCallback),
            npotTexturesSupported(false),
            anisotropicFilteringSupported(false),
            renderTargetsSupported(false),
            clampToBorderSupported(false),
            multisamplingSupported(false),
            uintIndicesSupported(false)
        {
        }

        void RenderDevice::init(Window* newWindow,
                                const Size2U&,
                                std::uint32_t newSampleCount,
                                bool newSrgb,
                                bool newVerticalSync,
                                bool newDepth,
                                bool newStencil,
                                bool newDebugRenderer)
        {
            window = newWindow;
            sampleCount = newSampleCount;
            srgb = newSrgb;
            verticalSync = newVerticalSync;
            depth = newDepth;
            stencil = newStencil;
            debugRenderer = newDebugRenderer;

            previousFrameTime = std::chrono::steady_clock::now();
        }

        void RenderDevice::process()
        {
            Event event;
            event.type = Event::Type::frame;
            callback(event);

            auto currentTime = std::chrono::steady_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - previousFrameTime);
            previousFrameTime = currentTime;

            const float delta = diff.count() / 1000000000.0F;

            if (delta > 0.0F)
                currentFPS = 1.0F / delta;

            accumulatedTime += delta;
            currentAccumulatedFPS += 1.0F;

            if (accumulatedTime > 1.0F)
            {
                accumulatedFPS = currentAccumulatedFPS;
                accumulatedTime = 0.0F;
                currentAccumulatedFPS = 0.0F;
            }
        }

        std::vector<Size2U> RenderDevice::getSupportedResolutions() const
        {
            return std::vector<Size2U>();
        }

        void RenderDevice::generateScreenshot(const std::string&)
        {
        }

        void RenderDevice::executeOnRenderThread(const std::function<void()>& func)
        {
            std::lock_guard<std::mutex> lock(executeMutex);
            executeQueue.push(func);
        }

        void RenderDevice::executeAll()
        {
            std::function<void()> func;

            for (;;)
            {
                std::unique_lock<std::mutex> lock(executeMutex);
                if (executeQueue.empty()) break;

                func = std::move(executeQueue.front());
                executeQueue.pop();
                lock.unlock();

                if (func) func();
            }
        }
    } // namespace graphics
} // namespace ouzel
