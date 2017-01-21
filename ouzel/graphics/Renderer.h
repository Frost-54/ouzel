// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <queue>
#include <set>
#include <memory>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "utils/Types.h"
#include "utils/Noncopyable.h"
#include "math/Rectangle.h"
#include "math/Matrix4.h"
#include "math/Size2.h"
#include "math/Color.h"
#include "math/AABB2.h"
#include "graphics/Vertex.h"
#include "graphics/BlendState.h"
#include "graphics/PixelFormat.h"
#include "graphics/Texture.h"

namespace ouzel
{
    class Engine;
    class Window;

    namespace graphics
    {
        const std::string SHADER_TEXTURE = "shaderTexture";
        const std::string SHADER_COLOR = "shaderColor";

        const std::string BLEND_NO_BLEND = "blendNoBlend";
        const std::string BLEND_ADD = "blendAdd";
        const std::string BLEND_MULTIPLY = "blendMultiply";
        const std::string BLEND_ALPHA = "blendAlpha";

        const std::string TEXTURE_WHITE_PIXEL = "textureWhitePixel";

        class Resource;

        class Renderer: public Noncopyable
        {
            friend Engine;
            friend Window;
        public:
            enum class Driver
            {
                DEFAULT,
                EMPTY,
                OPENGL,
                DIRECT3D11,
                METAL
            };

            enum class DrawMode
            {
                POINT_LIST,
                LINE_LIST,
                LINE_STRIP,
                TRIANGLE_LIST,
                TRIANGLE_STRIP
            };

            virtual ~Renderer();
            virtual void free();

            Driver getDriver() const { return driver; }

            void setClearColorBuffer(bool clear);
            bool getClearColorBuffer() const { return clearColorBuffer; }

            void setClearDepthBuffer(bool clear);
            bool getClearDepthBuffer() const { return clearDepthBuffer; }

            void setClearColor(Color color);
            Color getClearColor() const { return clearColor; }

            virtual bool present();

            virtual void setSize(const Size2& newSize);
            const Size2& getSize() const { return size; }
            uint32_t getSampleCount() const { return sampleCount; }
            Texture::Filter getTextureFilter() const { return textureFilter; }

            virtual std::vector<Size2> getSupportedResolutions() const;

            virtual BlendStateResourcePtr createBlendState() = 0;
            virtual TextureResourcePtr createTexture() = 0;
            virtual ShaderResourcePtr createShader() = 0;
            virtual MeshBufferResourcePtr createMeshBuffer() = 0;
            virtual IndexBufferResourcePtr createIndexBuffer() = 0;
            virtual VertexBufferResourcePtr createVertexBuffer() = 0;

            bool getRefillDrawQueue() const { return refillDrawQueue; }
            bool addDrawCommand(const std::vector<TextureResourcePtr>& textures,
                                const ShaderResourcePtr& shader,
                                const std::vector<std::vector<float>>& pixelShaderConstants,
                                const std::vector<std::vector<float>>& vertexShaderConstants,
                                const BlendStateResourcePtr& blendState,
                                const MeshBufferResourcePtr& meshBuffer,
                                uint32_t indexCount = 0,
                                DrawMode drawMode = DrawMode::TRIANGLE_LIST,
                                uint32_t startIndex = 0,
                                const TextureResourcePtr& renderTarget = nullptr,
                                const Rectangle& viewport = Rectangle(0.0f, 0.0f, 1.0f, 1.0f),
                                bool depthWrite = false,
                                bool depthTest = false,
                                bool wireframe = false,
                                bool scissorTestEnabled = false,
                                const Rectangle& scissorTest = Rectangle());
            void flushDrawCommands();

            Vector2 convertScreenToNormalizedLocation(const Vector2& position)
            {
                return Vector2(position.v[0] / size.v[0],
                               1.0f - (position.v[1] / size.v[1]));
            }

            Vector2 convertNormalizedToScreenLocation(const Vector2& position)
            {
                return Vector2(position.v[0] * size.v[0],
                               (1.0f - position.v[1]) * size.v[1]);
            }

            virtual bool saveScreenshot(const std::string& filename);

            virtual uint32_t getDrawCallCount() const { return drawCallCount; }

            uint16_t getAPIMajorVersion() const { return apiMajorVersion; }
            uint16_t getAPIMinorVersion() const { return apiMinorVersion; }

            void setAPIVersion(uint16_t majorVersion, uint16_t minorVersion)
            {
                apiMajorVersion = majorVersion;
                apiMinorVersion = minorVersion;
            }

            bool isNPOTTexturesSupported() const { return npotTexturesSupported; }
            bool isMultisamplingSupported() const { return multisamplingSupported; }

            const Matrix4& getProjectionTransform(bool renderTarget) const
            {
                return renderTarget ? renderTargetProjectionTransform : projectionTransform;
            }

        protected:
            Renderer(Driver aDriver);
            virtual bool init(Window* newWindow,
                              const Size2& newSize,
                              uint32_t newSampleCount,
                              Texture::Filter newTextureFilter,
                              PixelFormat newBackBufferFormat,
                              bool newVerticalSync,
                              bool newDepth);

            virtual bool update();

            bool generateScreenshots();
            virtual bool generateScreenshot(const std::string& filename);

            Driver driver;
            Window* window;

            uint16_t apiMajorVersion = 0;
            uint16_t apiMinorVersion = 0;

            uint32_t currentFrame = 0;
            uint32_t frameBufferClearedFrame = 0;
            uint32_t sampleCount = 1; // MSAA sample count
            Texture::Filter textureFilter = Texture::Filter::NONE;
            PixelFormat backBufferFormat;
            bool depth = false;

            bool verticalSync = true;

            struct DrawCommand
            {
                std::vector<TextureResourcePtr> textures;
                ShaderResourcePtr shader;
                std::vector<std::vector<float>> pixelShaderConstants;
                std::vector<std::vector<float>> vertexShaderConstants;
                BlendStateResourcePtr blendState;
                MeshBufferResourcePtr meshBuffer;
                uint32_t indexCount;
                DrawMode drawMode;
                uint32_t startIndex;
                TextureResourcePtr renderTarget;
                Rectangle viewport;
                bool depthWrite;
                bool depthTest;
                bool wireframe;
                bool scissorTestEnabled;
                Rectangle scissorTest;
            };

            bool npotTexturesSupported = true;
            bool multisamplingSupported = true;

            std::mutex drawQueueMutex;
            std::condition_variable drawQueueCondition;
            bool activeDrawQueueFinished = false;
            std::atomic<bool> refillDrawQueue;

            std::vector<DrawCommand> drawQueue;

            Matrix4 projectionTransform;
            Matrix4 renderTargetProjectionTransform;

            struct Data
            {
                Size2 size;
                Color clearColor;
                bool clearColorBuffer;
                bool clearDepthBuffer;
            };

            Data uploadData;

        private:
            Size2 size;

            Color clearColor;
            uint32_t drawCallCount = 0;

            bool clearColorBuffer = true;
            bool clearDepthBuffer = false;

            std::vector<DrawCommand> activeDrawQueue;

            std::set<Resource*> uploadSet;
            std::mutex uploadMutex;

            std::queue<std::string> screenshotQueue;
            std::mutex screenshotMutex;
            std::atomic<bool> dirty;

            static std::queue<Resource*> deleteQueue;
            static std::mutex deleteMutex;
        };
    } // namespace graphics
} // namespace ouzel
