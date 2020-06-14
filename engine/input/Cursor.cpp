// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <stdexcept>
#include "Cursor.hpp"
#include "InputManager.hpp"
#include "../core/Engine.hpp"
#include "stb_image.h"

namespace ouzel
{
    namespace input
    {
        Cursor::Cursor(InputManager& initInputManager):
            inputManager(initInputManager)
        {
            cursorResource = inputManager.getInputSystem()->getResourceId();
        }

        Cursor::Cursor(InputManager& initInputManager, SystemCursor systemCursor):
            Cursor(initInputManager)
        {
            init(systemCursor);
        }

        Cursor::Cursor(InputManager& initInputManager, const std::string& filename, const Vector2F& hotSpot):
            Cursor(initInputManager)
        {
            init(filename, hotSpot);
        }

        Cursor::~Cursor()
        {
            if (cursorResource)
            {
                InputSystem::Command command(InputSystem::Command::Type::destroyCursor);
                command.cursorResource = cursorResource;
                inputManager.getInputSystem()->addCommand(command);

                inputManager.getInputSystem()->deleteResourceId(cursorResource);
            }
        }

        void Cursor::init(SystemCursor systemCursor)
        {
            InputSystem::Command command(InputSystem::Command::Type::initCursor);
            command.cursorResource = cursorResource;
            command.systemCursor = systemCursor;
            inputManager.getInputSystem()->addCommand(command);
        }

        void Cursor::init(const std::string& filename, const Vector2F& hotSpot)
        {
            // TODO: load with asset loader
            const auto data = engine->getFileSystem().readFile(filename);

            int width;
            int height;
            int comp;

            stbi_uc* tempData = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(data.data()),
                                                      static_cast<int>(data.size()),
                                                      &width,
                                                      &height,
                                                      &comp,
                                                      STBI_default);

            if (!tempData)
                throw std::runtime_error("Failed to load texture, reason: " + std::string(stbi_failure_reason()));

            std::size_t pixelSize;
            graphics::PixelFormat pixelFormat;
            std::vector<std::uint8_t> imageData;

            switch (comp)
            {
                case STBI_grey:
                {
                    pixelFormat = graphics::PixelFormat::rgba8UnsignedNorm;
                    pixelSize = 4;

                    imageData.resize(static_cast<std::size_t>(width * height * 4));

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            const auto sourceOffset = static_cast<std::size_t>(y * height + x);
                            const auto destinationOffset = static_cast<std::size_t>((y * height + x) * 4);
                            imageData[destinationOffset + 0] = tempData[sourceOffset];
                            imageData[destinationOffset + 1] = tempData[sourceOffset];
                            imageData[destinationOffset + 2] = tempData[sourceOffset];
                            imageData[destinationOffset + 3] = 255;
                        }
                    }
                    stbi_image_free(tempData);
                    break;
                }
                case STBI_grey_alpha:
                {
                    pixelFormat = graphics::PixelFormat::rgba8UnsignedNorm;
                    pixelSize = 4;

                    imageData.resize(static_cast<std::size_t>(width * height * 4));

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            const auto sourceOffset = static_cast<std::size_t>((y * height + x) * 2);
                            const auto destinationOffset = static_cast<std::size_t>((y * height + x) * 4);
                            imageData[destinationOffset + 0] = tempData[sourceOffset + 0];
                            imageData[destinationOffset + 1] = tempData[sourceOffset + 0];
                            imageData[destinationOffset + 2] = tempData[sourceOffset + 0];
                            imageData[destinationOffset + 3] = tempData[sourceOffset + 1];
                        }
                    }
                    stbi_image_free(tempData);
                    break;
                }
                case STBI_rgb:
                {
                    pixelFormat = graphics::PixelFormat::rgba8UnsignedNorm;
                    pixelSize = 4;

                    imageData.resize(static_cast<std::size_t>(width * height * 4));

                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            const auto sourceOffset = static_cast<std::size_t>((y * height + x) * 3);
                            const auto destinationOffset = static_cast<std::size_t>((y * height + x) * 4);
                            imageData[destinationOffset + 0] = tempData[sourceOffset + 0];
                            imageData[destinationOffset + 1] = tempData[sourceOffset + 1];
                            imageData[destinationOffset + 2] = tempData[sourceOffset + 2];
                            imageData[destinationOffset + 3] = 255;
                        }
                    }
                    stbi_image_free(tempData);
                    break;
                }
                case STBI_rgb_alpha:
                {
                    pixelFormat = graphics::PixelFormat::rgba8UnsignedNorm;
                    pixelSize = 4;
                    imageData.assign(tempData,
                                     tempData + static_cast<std::size_t>(width * height) * pixelSize);
                    stbi_image_free(tempData);
                    break;
                }
                default:
                    stbi_image_free(tempData);
                    throw std::runtime_error("Unsupported pixel size");
            }

            init(imageData,
                 Size2F(static_cast<float>(width), static_cast<float>(height)),
                 pixelFormat,
                 hotSpot);
        }

        void Cursor::init(const std::vector<std::uint8_t>& data,
                          const Size2F& size,
                          graphics::PixelFormat pixelFormat,
                          const Vector2F& hotSpot)
        {
            InputSystem::Command command(InputSystem::Command::Type::initCursor);
            command.cursorResource = cursorResource;
            command.data = data;
            command.size = size;
            command.pixelFormat = pixelFormat;
            command.hotSpot = hotSpot;
            inputManager.getInputSystem()->addCommand(command);
        }
    } // namespace input
} // namespace ouzel
