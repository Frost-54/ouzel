// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "LoaderImage.hpp"
#include "Cache.hpp"
#include "graphics/ImageDataSTB.hpp"
#include "graphics/Texture.hpp"
#define STBI_NO_PSD
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_GIF
#define STBI_NO_PNM
#include "stb_image.h"

namespace ouzel
{
    namespace assets
    {
        LoaderImage::LoaderImage():
            Loader({"jpg", "jpeg", "png", "bmp", "tga"})
        {
            
        }

        bool LoaderImage::loadAsset(const std::string& filename)
        {
            graphics::ImageDataSTB image;
            if (!image.init(filename))
            {
                return false;
            }

            std::shared_ptr<graphics::Texture> texture(new graphics::Texture());

            if (texture->init(image.getData(), image.getSize(), 0, 0, image.getPixelFormat()))
            {
                return false;
            }

            cache->setTexture(filename, texture);

            return true;
        }
    } // namespace assets
} // namespace ouzel
