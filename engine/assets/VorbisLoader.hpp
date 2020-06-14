// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_ASSETS_VORBISLOADER_HPP
#define OUZEL_ASSETS_VORBISLOADER_HPP

#include "Loader.hpp"

namespace ouzel
{
    namespace assets
    {
        class VorbisLoader final: public Loader
        {
        public:
            explicit VorbisLoader(Cache& initCache);
            bool loadAsset(Bundle& bundle,
                           const std::string& name,
                           const std::vector<std::byte>& data,
                           bool mipmaps = true) final;
        };
    } // namespace assets
} // namespace ouzel

#endif // OUZEL_ASSETS_VORBISLOADER_HPP
