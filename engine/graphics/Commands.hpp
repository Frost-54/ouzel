// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_COMMANDS_HPP
#define OUZEL_GRAPHICS_COMMANDS_HPP

#include <queue>
#include <string>
#include "BlendFactor.hpp"
#include "BlendOperation.hpp"
#include "BufferType.hpp"
#include "ColorMask.hpp"
#include "CompareFunction.hpp"
#include "CubeFace.hpp"
#include "DataType.hpp"
#include "DrawMode.hpp"
#include "Flags.hpp"
#include "PixelFormat.hpp"
#include "RasterizerState.hpp"
#include "SamplerFilter.hpp"
#include "SamplerAddressMode.hpp"
#include "StencilOperation.hpp"
#include "TextureType.hpp"
#include "Vertex.hpp"
#include "../math/Color.hpp"
#include "../math/Rect.hpp"

namespace ouzel::graphics
{
    using ResourceId = std::size_t;

    class Command
    {
    public:
        enum class Type
        {
            resize,
            present,
            deleteResource,
            initRenderTarget,
            setRenderTarget,
            clearRenderTarget,
            setScissorTest,
            setViewport,
            initDepthStencilState,
            setDepthStencilState,
            setPipelineState,
            draw,
            initBlendState,
            initBuffer,
            setBufferData,
            initShader,
            setShaderConstants,
            initTexture,
            setTextureData,
            setTextureParameters,
            setTextures,

            initMaterial,
            setMaterialParameter,
            initObject,
            renderObject,
            setObjectParameters,
            initCamera,
            setCameraParameters,
            initLight,
            setLightParameters
        };

        explicit constexpr Command(Type initType) noexcept:
            type(initType)
        {
        }

        virtual ~Command() = default;

        const Type type;
    };

    class ResizeCommand final: public Command
    {
    public:
        explicit constexpr ResizeCommand(const Size<std::uint32_t, 2>& initSize) noexcept:
            Command(Command::Type::resize),
            size(initSize)
        {}

        const Size<std::uint32_t, 2> size;
    };

    class PresentCommand final: public Command
    {
    public:
        constexpr PresentCommand() noexcept:
            Command(Command::Type::present)
        {}
    };

    class DeleteResourceCommand final: public Command
    {
    public:
        explicit constexpr DeleteResourceCommand(ResourceId initResource) noexcept:
            Command(Command::Type::deleteResource),
            resource(initResource)
        {}

        const ResourceId resource;
    };

    class InitRenderTargetCommand final: public Command
    {
    public:
        InitRenderTargetCommand(ResourceId initRenderTarget,
                                const std::set<std::size_t>& initColorTextures,
                                ResourceId initDepthTexture) noexcept(false):
            Command(Command::Type::initRenderTarget),
            renderTarget(initRenderTarget),
            colorTextures(initColorTextures),
            depthTexture(initDepthTexture)
        {
        }

        const ResourceId renderTarget;
        const std::set<std::size_t> colorTextures;
        const ResourceId depthTexture;
    };

    class SetRenderTargetCommand final: public Command
    {
    public:
        explicit constexpr SetRenderTargetCommand(ResourceId initRenderTarget) noexcept:
            Command(Command::Type::setRenderTarget),
            renderTarget(initRenderTarget)
        {
        }

        const ResourceId renderTarget;
    };

    class ClearRenderTargetCommand final: public Command
    {
    public:
        constexpr ClearRenderTargetCommand(bool initClearColorBuffer,
                                           bool initClearDepthBuffer,
                                           bool initClearStencilBuffer,
                                           Color initClearColor,
                                           float initClearDepth,
                                           std::uint32_t initClearStencil) noexcept:
            Command(Command::Type::clearRenderTarget),
            clearColorBuffer(initClearColorBuffer),
            clearDepthBuffer(initClearDepthBuffer),
            clearStencilBuffer(initClearStencilBuffer),
            clearColor(initClearColor),
            clearDepth(initClearDepth),
            clearStencil(initClearStencil)
        {
        }

        const bool clearColorBuffer;
        const bool clearDepthBuffer;
        const bool clearStencilBuffer;
        const Color clearColor;
        const float clearDepth;
        const std::uint32_t clearStencil;
    };

    class SetScissorTestCommand final: public Command
    {
    public:
        constexpr SetScissorTestCommand(bool initEnabled,
                                        const Rect<float>& initRectangle) noexcept:
            Command(Command::Type::setScissorTest),
            enabled(initEnabled),
            rectangle(initRectangle)
        {
        }

        const bool enabled;
        const Rect<float> rectangle;
    };

    class SetViewportCommand final: public Command
    {
    public:
        explicit constexpr SetViewportCommand(const Rect<float>& initViewport) noexcept:
            Command(Command::Type::setViewport),
            viewport(initViewport)
        {
        }

        const Rect<float> viewport;
    };

    class InitDepthStencilStateCommand final: public Command
    {
    public:
        constexpr InitDepthStencilStateCommand(ResourceId initDepthStencilState,
                                               bool initDepthTest,
                                               bool initDepthWrite,
                                               CompareFunction initCompareFunction,
                                               bool initStencilEnabled,
                                               std::uint32_t initStencilReadMask,
                                               std::uint32_t initStencilWriteMask,
                                               StencilOperation initFrontFaceStencilFailureOperation,
                                               StencilOperation initFrontFaceStencilDepthFailureOperation,
                                               StencilOperation initFrontFaceStencilPassOperation,
                                               CompareFunction initFrontFaceStencilCompareFunction,
                                               StencilOperation initBackFaceStencilFailureOperation,
                                               StencilOperation initBackFaceStencilDepthFailureOperation,
                                               StencilOperation initBackFaceStencilPassOperation,
                                               CompareFunction initBackFaceStencilCompareFunction) noexcept:
            Command(Command::Type::initDepthStencilState),
            depthStencilState(initDepthStencilState),
            depthTest(initDepthTest),
            depthWrite(initDepthWrite),
            compareFunction(initCompareFunction),
            stencilEnabled(initStencilEnabled),
            stencilReadMask(initStencilReadMask),
            stencilWriteMask(initStencilWriteMask),
            frontFaceStencilFailureOperation(initFrontFaceStencilFailureOperation),
            frontFaceStencilDepthFailureOperation(initFrontFaceStencilDepthFailureOperation),
            frontFaceStencilPassOperation(initFrontFaceStencilPassOperation),
            frontFaceStencilCompareFunction(initFrontFaceStencilCompareFunction),
            backFaceStencilFailureOperation(initBackFaceStencilFailureOperation),
            backFaceStencilDepthFailureOperation(initBackFaceStencilDepthFailureOperation),
            backFaceStencilPassOperation(initBackFaceStencilPassOperation),
            backFaceStencilCompareFunction(initBackFaceStencilCompareFunction)
        {
        }

        const ResourceId depthStencilState;
        const bool depthTest;
        const bool depthWrite;
        const CompareFunction compareFunction;
        const bool stencilEnabled;
        const std::uint32_t stencilReadMask;
        const std::uint32_t stencilWriteMask;
        const StencilOperation frontFaceStencilFailureOperation;
        const StencilOperation frontFaceStencilDepthFailureOperation;
        const StencilOperation frontFaceStencilPassOperation;
        const CompareFunction frontFaceStencilCompareFunction;
        const StencilOperation backFaceStencilFailureOperation;
        const StencilOperation backFaceStencilDepthFailureOperation;
        const StencilOperation backFaceStencilPassOperation;
        const CompareFunction backFaceStencilCompareFunction;
    };

    class SetDepthStencilStateCommand final: public Command
    {
    public:
        constexpr SetDepthStencilStateCommand(ResourceId initDepthStencilState,
                                              std::uint32_t initStencilReferenceValue) noexcept:
            Command(Command::Type::setDepthStencilState),
            depthStencilState(initDepthStencilState),
            stencilReferenceValue(initStencilReferenceValue)
        {
        }

        const ResourceId depthStencilState;
        const std::uint32_t stencilReferenceValue;
    };

    class SetPipelineStateCommand final: public Command
    {
    public:
        constexpr SetPipelineStateCommand(ResourceId initBlendState,
                                          ResourceId initShader,
                                          CullMode initCullMode,
                                          FillMode initFillMode) noexcept:
            Command(Command::Type::setPipelineState),
            blendState(initBlendState),
            shader(initShader),
            cullMode(initCullMode),
            fillMode(initFillMode)
        {
        }

        const ResourceId blendState;
        const ResourceId shader;
        const CullMode cullMode;
        const FillMode fillMode;
    };

    class DrawCommand final: public Command
    {
    public:
        constexpr DrawCommand(ResourceId initIndexBuffer,
                              std::uint32_t initIndexCount,
                              std::uint32_t initIndexSize,
                              ResourceId initVertexBuffer,
                              DrawMode initDrawMode,
                              std::uint32_t initStartIndex) noexcept:
            Command(Command::Type::draw),
            indexBuffer(initIndexBuffer),
            indexCount(initIndexCount),
            indexSize(initIndexSize),
            vertexBuffer(initVertexBuffer),
            drawMode(initDrawMode),
            startIndex(initStartIndex)
        {
        }

        const ResourceId indexBuffer;
        const std::uint32_t indexCount;
        const std::uint32_t indexSize;
        const ResourceId vertexBuffer;
        const DrawMode drawMode;
        const std::uint32_t startIndex;
    };

    class InitBlendStateCommand final: public Command
    {
    public:
        constexpr InitBlendStateCommand(ResourceId initBlendState,
                                        bool initEnableBlending,
                                        BlendFactor initColorBlendSource,
                                        BlendFactor initColorBlendDest,
                                        BlendOperation initColorOperation,
                                        BlendFactor initAlphaBlendSource,
                                        BlendFactor initAlphaBlendDest,
                                        BlendOperation initAlphaOperation,
                                        ColorMask initColorMask) noexcept:
            Command(Command::Type::initBlendState),
            blendState(initBlendState),
            enableBlending(initEnableBlending),
            colorBlendSource(initColorBlendSource),
            colorBlendDest(initColorBlendDest),
            colorOperation(initColorOperation),
            alphaBlendSource(initAlphaBlendSource),
            alphaBlendDest(initAlphaBlendDest),
            alphaOperation(initAlphaOperation),
            colorMask(initColorMask)
        {
        }

        const ResourceId blendState;
        const bool enableBlending;
        const BlendFactor colorBlendSource;
        const BlendFactor colorBlendDest;
        const BlendOperation colorOperation;
        const BlendFactor alphaBlendSource;
        const BlendFactor alphaBlendDest;
        const BlendOperation alphaOperation;
        const ColorMask colorMask;
    };

    class InitBufferCommand final: public Command
    {
    public:
        InitBufferCommand(ResourceId initBuffer,
                          BufferType initBufferType,
                          Flags initFlags,
                          const std::vector<std::uint8_t>& initData,
                          std::uint32_t initSize) noexcept(false):
            Command(Command::Type::initBuffer),
            buffer(initBuffer),
            bufferType(initBufferType),
            flags(initFlags),
            data(initData),
            size(initSize)
        {
        }

        const ResourceId buffer;
        const BufferType bufferType;
        const Flags flags;
        const std::vector<std::uint8_t> data;
        const std::uint32_t size;
    };

    class SetBufferDataCommand final: public Command
    {
    public:
        SetBufferDataCommand(ResourceId initBuffer,
                             const std::vector<std::uint8_t>& initData) noexcept(false):
            Command(Command::Type::setBufferData),
            buffer(initBuffer),
            data(initData)
        {
        }

        const ResourceId buffer;
        const std::vector<std::uint8_t> data;
    };

    class InitShaderCommand final: public Command
    {
    public:
        InitShaderCommand(ResourceId initShader,
                          const std::vector<std::uint8_t>& initFragmentShader,
                          const std::vector<std::uint8_t>& initVertexShader,
                          const std::set<Vertex::Attribute::Usage>& initVertexAttributes,
                          const std::vector<std::pair<std::string, DataType>>& initFragmentShaderConstantInfo,
                          const std::vector<std::pair<std::string, DataType>>& initVertexShaderConstantInfo,
                          const std::string& initFragmentShaderFunction,
                          const std::string& initVertexShaderFunction) noexcept(false):
            Command(Command::Type::initShader),
            shader(initShader),
            fragmentShader(initFragmentShader),
            vertexShader(initVertexShader),
            vertexAttributes(initVertexAttributes),
            fragmentShaderConstantInfo(initFragmentShaderConstantInfo),
            vertexShaderConstantInfo(initVertexShaderConstantInfo),
            fragmentShaderFunction(initFragmentShaderFunction),
            vertexShaderFunction(initVertexShaderFunction)
        {
        }

        const ResourceId shader;
        const std::vector<std::uint8_t> fragmentShader;
        const std::vector<std::uint8_t> vertexShader;
        const std::set<Vertex::Attribute::Usage> vertexAttributes;
        const std::vector<std::pair<std::string, DataType>> fragmentShaderConstantInfo;
        const std::vector<std::pair<std::string, DataType>> vertexShaderConstantInfo;
        const std::string fragmentShaderFunction;
        const std::string vertexShaderFunction;
    };

    class SetShaderConstantsCommand final: public Command
    {
    public:
        SetShaderConstantsCommand(std::vector<std::vector<float>> initFragmentShaderConstants,
                                  std::vector<std::vector<float>> initVertexShaderConstants) noexcept(false):
            Command(Command::Type::setShaderConstants),
            fragmentShaderConstants(initFragmentShaderConstants),
            vertexShaderConstants(initVertexShaderConstants)
        {
        }

        const std::vector<std::vector<float>> fragmentShaderConstants;
        const std::vector<std::vector<float>> vertexShaderConstants;
    };

    class InitTextureCommand final: public Command
    {
    public:
        InitTextureCommand(ResourceId initTexture,
                           const std::vector<std::pair<Size<std::uint32_t, 2>, std::vector<std::uint8_t>>>& initLevels,
                           TextureType initTextureType,
                           Flags initFlags,
                           std::uint32_t initSampleCount,
                           PixelFormat initPixelFormat,
                           SamplerFilter initFilter,
                           std::uint32_t initMaxAnisotropy) noexcept(false):
            Command(Command::Type::initTexture),
            texture(initTexture),
            levels(initLevels),
            textureType(initTextureType),
            flags(initFlags),
            sampleCount(initSampleCount),
            pixelFormat(initPixelFormat),
            filter(initFilter),
            maxAnisotropy(initMaxAnisotropy)
        {
        }

        const ResourceId texture;
        const std::vector<std::pair<Size<std::uint32_t, 2>, std::vector<std::uint8_t>>> levels;
        const TextureType textureType;
        const Flags flags;
        const std::uint32_t sampleCount;
        const PixelFormat pixelFormat;
        const SamplerFilter filter;
        const std::uint32_t maxAnisotropy;
    };

    class SetTextureDataCommand final: public Command
    {
    public:
        SetTextureDataCommand(ResourceId initTexture,
                              const std::vector<std::pair<Size<std::uint32_t, 2>, std::vector<std::uint8_t>>>& initLevels) noexcept(false):
            Command(Command::Type::setTextureData),
            texture(initTexture),
            levels(initLevels),
            face(CubeFace::positiveX)
        {
        }

        SetTextureDataCommand(ResourceId initTexture,
                              const std::vector<std::pair<Size<std::uint32_t, 2>, std::vector<std::uint8_t>>>& initLevels,
                              CubeFace initFace) noexcept(false):
            Command(Command::Type::setTextureData),
            texture(initTexture),
            levels(initLevels),
            face(initFace)
        {
        }

        const ResourceId texture;
        const std::vector<std::pair<Size<std::uint32_t, 2>, std::vector<std::uint8_t>>> levels;
        const CubeFace face;
    };

    class SetTextureParametersCommand final: public Command
    {
    public:
        constexpr SetTextureParametersCommand(ResourceId initTexture,
                                              SamplerFilter initFilter,
                                              SamplerAddressMode initAddressX,
                                              SamplerAddressMode initAddressY,
                                              SamplerAddressMode initAddressZ,
                                              Color initBorderColor,
                                              std::uint32_t initMaxAnisotropy) noexcept:
            Command(Command::Type::setTextureParameters),
            texture(initTexture),
            filter(initFilter),
            addressX(initAddressX),
            addressY(initAddressY),
            addressZ(initAddressZ),
            borderColor(initBorderColor),
            maxAnisotropy(initMaxAnisotropy)
        {
        }

        const ResourceId texture;
        const SamplerFilter filter;
        const SamplerAddressMode addressX;
        const SamplerAddressMode addressY;
        const SamplerAddressMode addressZ;
        const Color borderColor;
        const std::uint32_t maxAnisotropy;
    };

    class SetTexturesCommand final: public Command
    {
    public:
        explicit SetTexturesCommand(const std::vector<ResourceId>& initTextures) noexcept(false):
            Command(Command::Type::setTextures),
            textures(initTextures)
        {
        }

        const std::vector<ResourceId> textures;
    };

    class CommandBuffer final
    {
    public:
        CommandBuffer() = default;
        explicit CommandBuffer(const std::string& initName) noexcept(false):
            name(initName)
        {
        }

        auto& getName() const noexcept { return name; }

        auto isEmpty() const noexcept { return commands.empty(); }

        void pushCommand(std::unique_ptr<Command> command)
        {
            commands.push(std::move(command));
        }

        std::unique_ptr<Command> popCommand()
        {
            auto result = std::move(commands.front());
            commands.pop();
            return result;
        }

        auto& getCommands() const
        {
            return commands;
        }

    private:
        std::string name;
        std::queue<std::unique_ptr<Command>> commands;
    };
}

#endif // OUZEL_GRAPHICS_COMMANDS_HPP
