// Ouzel by Elviss Strazdins

#ifndef OUZEL_ASSETS_PARTICLESYSTEMLOADER_HPP
#define OUZEL_ASSETS_PARTICLESYSTEMLOADER_HPP

#include "Loader.hpp"

namespace ouzel::assets
{
    class ParticleSystemLoader final: public Loader
    {
    public:
        explicit ParticleSystemLoader(Cache& initCache);
        bool loadAsset(Bundle& bundle,
                       const std::string& name,
                       const std::vector<std::byte>& data,
                       bool mipmaps = true) override;
    };
}

#endif // OUZEL_ASSETS_PARTICLESYSTEMLOADER_HPP
