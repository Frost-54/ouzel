// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif
#include <algorithm>
#include <stdexcept>
#include "Setup.h"
#include "Engine.hpp"
#include "../utils/Utils.hpp"
#include "../graphics/Renderer.hpp"
#include "../audio/Audio.hpp"

#if OUZEL_COMPILE_OPENGL
#  include "../graphics/opengl/OGL.h"
#  if OUZEL_OPENGLES
#    include "opengl/ColorPSGLES2.h"
#    include "opengl/ColorVSGLES2.h"
#    include "opengl/TexturePSGLES2.h"
#    include "opengl/TextureVSGLES2.h"
#    include "opengl/ColorPSGLES3.h"
#    include "opengl/ColorVSGLES3.h"
#    include "opengl/TexturePSGLES3.h"
#    include "opengl/TextureVSGLES3.h"
#  else
#    include "opengl/ColorPSGL2.h"
#    include "opengl/ColorVSGL2.h"
#    include "opengl/TexturePSGL2.h"
#    include "opengl/TextureVSGL2.h"
#    include "opengl/ColorPSGL3.h"
#    include "opengl/ColorVSGL3.h"
#    include "opengl/TexturePSGL3.h"
#    include "opengl/TextureVSGL3.h"
#    include "opengl/ColorPSGL4.h"
#    include "opengl/ColorVSGL4.h"
#    include "opengl/TexturePSGL4.h"
#    include "opengl/TextureVSGL4.h"
#  endif
#endif

#if OUZEL_COMPILE_DIRECT3D11
#  include "direct3d11/TexturePSD3D11.h"
#  include "direct3d11/TextureVSD3D11.h"
#  include "direct3d11/ColorPSD3D11.h"
#  include "direct3d11/ColorVSD3D11.h"
#endif

#if OUZEL_COMPILE_METAL
#  if TARGET_OS_IOS
#    include "metal/ColorPSIOS.h"
#    include "metal/ColorVSIOS.h"
#    include "metal/TexturePSIOS.h"
#    include "metal/TextureVSIOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSIOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSIOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSIOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSIOS_metallib
#  elif TARGET_OS_TV
#    include "metal/ColorPSTVOS.h"
#    include "metal/ColorVSTVOS.h"
#    include "metal/TexturePSTVOS.h"
#    include "metal/TextureVSTVOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSTVOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSTVOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSTVOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSTVOS_metallib
#  elif TARGET_OS_MAC
#    include "metal/ColorPSMacOS.h"
#    include "metal/ColorVSMacOS.h"
#    include "metal/TexturePSMacOS.h"
#    include "metal/TextureVSMacOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSMacOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSMacOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSMacOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSMacOS_metallib
#  endif
#endif

namespace ouzel
{
    core::Engine* engine = nullptr;
}

namespace ouzel::core
{
    Engine::Engine():
        fileSystem(*this),
        assetBundle(cache, fileSystem)
    {
        engine = this;
    }

    Engine::~Engine()
    {
        if (active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::engineStop;
            eventDispatcher.postEvent(std::move(event));
        }

        paused = true;
        active = false;

#if !defined(__EMSCRIPTEN__)
        if (updateThread.isJoinable())
        {
            std::unique_lock lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif
    }

    void Engine::init()
    {
        Thread::setCurrentThreadName("Main");

        Size2U size;
        std::uint32_t sampleCount = 1; // MSAA sample count
        graphics::SamplerFilter textureFilter = graphics::SamplerFilter::point;
        std::uint32_t maxAnisotropy = 1;
        bool resizable = false;
        bool fullscreen = false;
        bool verticalSync = true;
        bool depth = false;
        bool stencil = false;
        bool debugRenderer = false;
        bool exclusiveFullscreen = false;
        bool highDpi = true; // should high DPI resolution be used
        bool debugAudio = false;

        if (fileSystem.resourceFileExists("settings.ini"))
            defaultSettings = ini::parse(fileSystem.readFile("settings.ini"));

        auto settingsPath = fileSystem.getStorageDirectory() / "settings.ini";
        if (fileSystem.fileExists(settingsPath))
            userSettings = ini::parse(fileSystem.readFile(settingsPath));

        const ini::Section& userEngineSection = userSettings["engine"];
        const ini::Section& defaultEngineSection = defaultSettings["engine"];

        const std::string graphicsDriverValue = userEngineSection.getValue("graphicsDriver", defaultEngineSection.getValue("graphicsDriver"));

        const std::string widthValue = userEngineSection.getValue("width", defaultEngineSection.getValue("width"));
        if (!widthValue.empty()) size.v[0] = static_cast<std::uint32_t>(std::stoul(widthValue));

        const std::string heightValue = userEngineSection.getValue("height", defaultEngineSection.getValue("height"));
        if (!heightValue.empty()) size.v[1] = static_cast<std::uint32_t>(std::stoul(heightValue));

        const std::string sampleCountValue = userEngineSection.getValue("sampleCount", defaultEngineSection.getValue("sampleCount"));
        if (!sampleCountValue.empty()) sampleCount = static_cast<std::uint32_t>(std::stoul(sampleCountValue));

        const std::string textureFilterValue = userEngineSection.getValue("textureFilter", defaultEngineSection.getValue("textureFilter"));
        if (!textureFilterValue.empty())
        {
            if (textureFilterValue == "point")
                textureFilter = graphics::SamplerFilter::point;
            else if (textureFilterValue == "linear")
                textureFilter = graphics::SamplerFilter::linear;
            else if (textureFilterValue == "bilinear")
                textureFilter = graphics::SamplerFilter::bilinear;
            else if (textureFilterValue == "trilinear")
                textureFilter = graphics::SamplerFilter::trilinear;
            else
                throw std::runtime_error("Invalid texture filter specified");
        }

        const std::string maxAnisotropyValue = userEngineSection.getValue("maxAnisotropy", defaultEngineSection.getValue("maxAnisotropy"));
        if (!maxAnisotropyValue.empty()) maxAnisotropy = static_cast<std::uint32_t>(std::stoul(maxAnisotropyValue));

        const std::string resizableValue = userEngineSection.getValue("resizable", defaultEngineSection.getValue("resizable"));
        if (!resizableValue.empty()) resizable = (resizableValue == "true" || resizableValue == "1" || resizableValue == "yes");

        const std::string fullscreenValue = userEngineSection.getValue("fullscreen", defaultEngineSection.getValue("fullscreen"));
        if (!fullscreenValue.empty()) fullscreen = (fullscreenValue == "true" || fullscreenValue == "1" || fullscreenValue == "yes");

        const std::string verticalSyncValue = userEngineSection.getValue("verticalSync", defaultEngineSection.getValue("verticalSync"));
        if (!verticalSyncValue.empty()) verticalSync = (verticalSyncValue == "true" || verticalSyncValue == "1" || verticalSyncValue == "yes");

        const std::string exclusiveFullscreenValue = userEngineSection.getValue("exclusiveFullscreen", defaultEngineSection.getValue("exclusiveFullscreen"));
        if (!exclusiveFullscreenValue.empty()) exclusiveFullscreen = (exclusiveFullscreenValue == "true" || exclusiveFullscreenValue == "1" || exclusiveFullscreenValue == "yes");

        const std::string depthValue = userEngineSection.getValue("depth", defaultEngineSection.getValue("depth"));
        if (!depthValue.empty()) depth = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        const std::string stencilValue = userEngineSection.getValue("stencil", defaultEngineSection.getValue("stencil"));
        if (!stencilValue.empty()) stencil = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        const std::string debugRendererValue = userEngineSection.getValue("debugRenderer", defaultEngineSection.getValue("debugRenderer"));
        if (!debugRendererValue.empty()) debugRenderer = (debugRendererValue == "true" || debugRendererValue == "1" || debugRendererValue == "yes");

        const std::string highDpiValue = userEngineSection.getValue("highDpi", defaultEngineSection.getValue("highDpi"));
        if (!highDpiValue.empty()) highDpi = (highDpiValue == "true" || highDpiValue == "1" || highDpiValue == "yes");

        const std::string audioDriverValue = userEngineSection.getValue("audioDriver", defaultEngineSection.getValue("audioDriver"));

        const std::string debugAudioValue = userEngineSection.getValue("debugAudio", defaultEngineSection.getValue("debugAudio"));
        if (!debugAudioValue.empty()) debugAudio = (debugAudioValue == "true" || debugAudioValue == "1" || debugAudioValue == "yes");

        graphics::Driver graphicsDriver = graphics::Renderer::getDriver(graphicsDriverValue);

        const Window::Flags windowFlags =
            (resizable ? Window::Flags::resizable : Window::Flags::none) |
            (fullscreen ? Window::Flags::fullscreen : Window::Flags::none) |
            (exclusiveFullscreen ? Window::Flags::exclusiveFullscreen : Window::Flags::none) |
            (highDpi ? Window::Flags::highDpi : Window::Flags::none) |
            (depth ? Window::Flags::depth : Window::Flags::none);

        window = std::make_unique<Window>(*this,
                                          size,
                                          windowFlags,
                                          OUZEL_APPLICATION_NAME,
                                          graphicsDriver);

        renderer = std::make_unique<graphics::Renderer>(graphicsDriver,
                                                        window.get(),
                                                        window->getResolution(),
                                                        sampleCount,
                                                        textureFilter,
                                                        maxAnisotropy,
                                                        false,
                                                        verticalSync,
                                                        depth,
                                                        stencil,
                                                        debugRenderer);

        audio::Driver audioDriver = audio::Audio::getDriver(audioDriverValue);
        audio = std::make_unique<audio::Audio>(audioDriver, debugAudio);

        inputManager = std::make_unique<input::InputManager>();

        // default assets
        switch (graphicsDriver)
        {
#if OUZEL_COMPILE_OPENGL
            case graphics::Driver::openGL:
            {
                std::unique_ptr<graphics::Shader> textureShader;

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_OPENGLES
                    case 2:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<std::uint8_t>(std::begin(TexturePSGLES2_glsl),
                                                                                                std::end(TexturePSGLES2_glsl)),
                                                                           std::vector<std::uint8_t>(std::begin(TextureVSGLES2_glsl),
                                                                                                std::end(TextureVSGLES2_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::position,
                                                                               graphics::Vertex::Attribute::Usage::color,
                                                                               graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color",graphics::DataType::float32Vector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                           });
                        break;
                    case 3:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<std::uint8_t>(std::begin(TexturePSGLES3_glsl),
                                                                                                std::end(TexturePSGLES3_glsl)),
                                                                           std::vector<std::uint8_t>(std::begin(TextureVSGLES3_glsl),
                                                                                                std::end(TextureVSGLES3_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::position,
                                                                               graphics::Vertex::Attribute::Usage::color,
                                                                               graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::float32Vector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                           });
                        break;
#  else
                    case 2:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<std::uint8_t>(std::begin(TexturePSGL2_glsl),
                                                                                                std::end(TexturePSGL2_glsl)),
                                                                           std::vector<std::uint8_t>(std::begin(TextureVSGL2_glsl),
                                                                                                std::end(TextureVSGL2_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::position,
                                                                               graphics::Vertex::Attribute::Usage::color,
                                                                               graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::float32Vector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                           });
                        break;
                    case 3:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<std::uint8_t>(std::begin(TexturePSGL3_glsl),
                                                                                                std::end(TexturePSGL3_glsl)),
                                                                           std::vector<std::uint8_t>(std::begin(TextureVSGL3_glsl),
                                                                                                std::end(TextureVSGL3_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::position,
                                                                               graphics::Vertex::Attribute::Usage::color,
                                                                               graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::float32Vector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                           });
                        break;
                    case 4:
                        textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                           std::vector<std::uint8_t>(std::begin(TexturePSGL4_glsl),
                                                                                                std::end(TexturePSGL4_glsl)),
                                                                           std::vector<std::uint8_t>(std::begin(TextureVSGL4_glsl),
                                                                                                std::end(TextureVSGL4_glsl)),
                                                                           std::set<graphics::Vertex::Attribute::Usage>{
                                                                               graphics::Vertex::Attribute::Usage::position,
                                                                               graphics::Vertex::Attribute::Usage::color,
                                                                               graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"color", graphics::DataType::float32Vector4}
                                                                           },
                                                                           std::vector<std::pair<std::string, graphics::DataType>>{
                                                                               {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                           });
                        break;
#  endif
                    default:
                        throw std::runtime_error("Unsupported OpenGL version");
                }

                assetBundle.setShader(shaderTexture, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer);

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_OPENGLES
                    case 2:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<std::uint8_t>(std::begin(ColorPSGLES2_glsl),
                                                                                              std::end(ColorPSGLES2_glsl)),
                                                                         std::vector<std::uint8_t>(std::begin(ColorVSGLES2_glsl),
                                                                                              std::end(ColorVSGLES2_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::position,
                                                                             graphics::Vertex::Attribute::Usage::color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::float32Vector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                         });
                        break;
                    case 3:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<std::uint8_t>(std::begin(ColorPSGLES3_glsl),
                                                                                              std::end(ColorPSGLES3_glsl)),
                                                                         std::vector<std::uint8_t>(std::begin(ColorVSGLES3_glsl),
                                                                                              std::end(ColorVSGLES3_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::position,
                                                                             graphics::Vertex::Attribute::Usage::color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::float32Vector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                         });
                        break;
#  else
                    case 2:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<std::uint8_t>(std::begin(ColorPSGL2_glsl),
                                                                                              std::end(ColorPSGL2_glsl)),
                                                                         std::vector<std::uint8_t>(std::begin(ColorVSGL2_glsl),
                                                                                              std::end(ColorVSGL2_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::position,
                                                                             graphics::Vertex::Attribute::Usage::color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::float32Vector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                         });
                        break;
                    case 3:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<std::uint8_t>(std::begin(ColorPSGL3_glsl),
                                                                                              std::end(ColorPSGL3_glsl)),
                                                                         std::vector<std::uint8_t>(std::begin(ColorVSGL3_glsl),
                                                                                              std::end(ColorVSGL3_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::position,
                                                                             graphics::Vertex::Attribute::Usage::color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::float32Vector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                         });
                        break;
                    case 4:
                        colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                         std::vector<std::uint8_t>(std::begin(ColorPSGL4_glsl),
                                                                                              std::end(ColorPSGL4_glsl)),
                                                                         std::vector<std::uint8_t>(std::begin(ColorVSGL4_glsl),
                                                                                              std::end(ColorVSGL4_glsl)),
                                                                         std::set<graphics::Vertex::Attribute::Usage>{
                                                                             graphics::Vertex::Attribute::Usage::position,
                                                                             graphics::Vertex::Attribute::Usage::color
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"color", graphics::DataType::float32Vector4}
                                                                         },
                                                                         std::vector<std::pair<std::string, graphics::DataType>>{
                                                                             {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                         });
                        break;
#  endif
                    default:
                        throw std::runtime_error("Unsupported OpenGL version");
                }

                assetBundle.setShader(shaderColor, std::move(colorShader));
                break;
            }
#endif

#if OUZEL_COMPILE_DIRECT3D11
            case graphics::Driver::direct3D11:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<std::uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_D3D11),
                                                                                             std::end(TEXTURE_PIXEL_SHADER_D3D11)),
                                                                        std::vector<std::uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_D3D11),
                                                                                             std::end(TEXTURE_VERTEX_SHADER_D3D11)),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::position,
                                                                            graphics::Vertex::Attribute::Usage::color,
                                                                            graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::float32Vector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                        });

                assetBundle.setShader(shaderTexture, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<std::uint8_t>(std::begin(COLOR_PIXEL_SHADER_D3D11),
                                                                                           std::end(COLOR_PIXEL_SHADER_D3D11)),
                                                                      std::vector<std::uint8_t>(std::begin(COLOR_VERTEX_SHADER_D3D11),
                                                                                           std::end(COLOR_VERTEX_SHADER_D3D11)),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::position,
                                                                          graphics::Vertex::Attribute::Usage::color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::float32Vector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                      });

                assetBundle.setShader(shaderColor, std::move(colorShader));
                break;
            }
#endif

#if OUZEL_COMPILE_METAL
            case graphics::Driver::metal:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<std::uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_METAL),
                                                                                             std::end(TEXTURE_PIXEL_SHADER_METAL)),
                                                                        std::vector<std::uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_METAL),
                                                                                             std::end(TEXTURE_VERTEX_SHADER_METAL)),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::position,
                                                                            graphics::Vertex::Attribute::Usage::color,
                                                                            graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::float32Vector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                        },
                                                                        "mainPS", "mainVS");

                assetBundle.setShader(shaderTexture, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<std::uint8_t>(std::begin(COLOR_PIXEL_SHADER_METAL),
                                                                                           std::end(COLOR_PIXEL_SHADER_METAL)),
                                                                      std::vector<std::uint8_t>(std::begin(COLOR_VERTEX_SHADER_METAL),
                                                                                           std::end(COLOR_VERTEX_SHADER_METAL)),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::position,
                                                                          graphics::Vertex::Attribute::Usage::color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::float32Vector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                      },
                                                                      "mainPS", "mainVS");

                assetBundle.setShader(shaderColor, std::move(colorShader));
                break;
            }
#endif

            default:
            {
                auto textureShader = std::make_unique<graphics::Shader>(*renderer,
                                                                        std::vector<std::uint8_t>(),
                                                                        std::vector<std::uint8_t>(),
                                                                        std::set<graphics::Vertex::Attribute::Usage>{
                                                                            graphics::Vertex::Attribute::Usage::position,
                                                                            graphics::Vertex::Attribute::Usage::color,
                                                                            graphics::Vertex::Attribute::Usage::textureCoordinates0
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"color", graphics::DataType::float32Vector4}
                                                                        },
                                                                        std::vector<std::pair<std::string, graphics::DataType>>{
                                                                            {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                        });

                assetBundle.setShader(shaderTexture, std::move(textureShader));

                auto colorShader = std::make_unique<graphics::Shader>(*renderer,
                                                                      std::vector<std::uint8_t>(),
                                                                      std::vector<std::uint8_t>(),
                                                                      std::set<graphics::Vertex::Attribute::Usage>{
                                                                          graphics::Vertex::Attribute::Usage::position,
                                                                          graphics::Vertex::Attribute::Usage::color
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"color", graphics::DataType::float32Vector4}
                                                                      },
                                                                      std::vector<std::pair<std::string, graphics::DataType>>{
                                                                          {"modelViewProj", graphics::DataType::float32Matrix4}
                                                                      });

                assetBundle.setShader(shaderColor, std::move(colorShader));
                break;
            }
        }

        auto noBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                   false,
                                                                   graphics::BlendFactor::one,
                                                                   graphics::BlendFactor::zero,
                                                                   graphics::BlendOperation::add,
                                                                   graphics::BlendFactor::one,
                                                                   graphics::BlendFactor::zero,
                                                                   graphics::BlendOperation::add);

        assetBundle.setBlendState(blendNoBlend, std::move(noBlendState));

        auto addBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                    true,
                                                                    graphics::BlendFactor::one,
                                                                    graphics::BlendFactor::one,
                                                                    graphics::BlendOperation::add,
                                                                    graphics::BlendFactor::one,
                                                                    graphics::BlendFactor::one,
                                                                    graphics::BlendOperation::add);

        assetBundle.setBlendState(blendAdd, std::move(addBlendState));

        auto multiplyBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                         true,
                                                                         graphics::BlendFactor::destColor,
                                                                         graphics::BlendFactor::zero,
                                                                         graphics::BlendOperation::add,
                                                                         graphics::BlendFactor::one,
                                                                         graphics::BlendFactor::one,
                                                                         graphics::BlendOperation::add);

        assetBundle.setBlendState(blendMultiply, std::move(multiplyBlendState));

        auto alphaBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                      true,
                                                                      graphics::BlendFactor::srcAlpha,
                                                                      graphics::BlendFactor::invSrcAlpha,
                                                                      graphics::BlendOperation::add,
                                                                      graphics::BlendFactor::one,
                                                                      graphics::BlendFactor::one,
                                                                      graphics::BlendOperation::add);

        assetBundle.setBlendState(blendAlpha, std::move(alphaBlendState));

        auto screenBlendState = std::make_unique<graphics::BlendState>(*renderer,
                                                                       true,
                                                                       graphics::BlendFactor::one,
                                                                       graphics::BlendFactor::invSrcAlpha,
                                                                       graphics::BlendOperation::add,
                                                                       graphics::BlendFactor::one,
                                                                       graphics::BlendFactor::one,
                                                                       graphics::BlendOperation::add);

        assetBundle.setBlendState(blendScreen, std::move(screenBlendState));

        auto whitePixelTexture = std::make_shared<graphics::Texture>(*renderer,
                                                                     std::vector<std::uint8_t>{255, 255, 255, 255},
                                                                     Size2U(1, 1),
                                                                     graphics::Flags::none, 1);
        assetBundle.setTexture(textureWhitePixel, whitePixelTexture);
    }

    void Engine::start()
    {
        if (!active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::engineStart;
            eventDispatcher.postEvent(std::move(event));

            active = true;
            paused = false;

#if !defined(__EMSCRIPTEN__)
            updateThread = Thread(&Engine::engineMain, this);
#else
            main(args);
#endif
        }
    }

    void Engine::pause()
    {
        if (active && !paused)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::enginePause;
            eventDispatcher.postEvent(std::move(event));

            paused = true;
        }
    }

    void Engine::resume()
    {
        if (active && paused)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::engineResume;
            eventDispatcher.postEvent(std::move(event));

            paused = false;

#if !defined(__EMSCRIPTEN__)
            updateCondition.notify_all();
#endif
        }
    }

    void Engine::exit()
    {
        paused = true;

        if (active)
        {
            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::engineStop;
            eventDispatcher.postEvent(std::move(event));

            active = false;
        }

#if !defined(__EMSCRIPTEN__)
        if (updateThread.isJoinable() &&
            updateThread.getId() != std::this_thread::get_id())
        {
            std::unique_lock lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif
    }

    void Engine::update()
    {
        eventDispatcher.dispatchEvents();

        const auto currentTime = std::chrono::steady_clock::now();
        auto diff = currentTime - previousUpdateTime;

        if (diff > std::chrono::milliseconds(1)) // at least one millisecond has passed
        {
            if (diff > std::chrono::seconds(1000 / 20)) diff = std::chrono::milliseconds(1000 / 20); // limit the update rate to a minimum 20 FPS

            previousUpdateTime = currentTime;
            const float delta = static_cast<float>(std::chrono::duration_cast<std::chrono::microseconds>(diff).count()) / 1000000.0F;

            auto updateEvent = std::make_unique<UpdateEvent>();
            updateEvent->type = Event::Type::update;
            updateEvent->delta = delta;
            eventDispatcher.dispatchEvent(std::move(updateEvent));
        }

        inputManager->update();
        window->update();
        audio->update();

        if (renderer->getRefillQueue())
            sceneManager.draw();

        if (oneUpdatePerFrame) renderer->waitForNextFrame();
    }

    void Engine::executeOnMainThread(const std::function<void()>& func)
    {
        if (active) runOnMainThread(func);
    }

    void Engine::engineMain()
    {
        Thread::setCurrentThreadName("Application");

        try
        {
            std::unique_ptr<Application> application = ouzel::main(args);

#if !defined(__EMSCRIPTEN__)
            while (active)
            {
                if (!paused)
                    update();
                else
                {
                    std::unique_lock lock(updateMutex);
                    while (active && paused)
                        updateCondition.wait(lock);
                }
            }

            eventDispatcher.dispatchEvents();
#endif
        }
        catch (const std::exception& e)
        {
            log(Log::Level::error) << e.what();
            exit();
        }
    }

    void Engine::openUrl(const std::string&)
    {
    }

    void Engine::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        screenSaverEnabled = newScreenSaverEnabled;
    }
}
