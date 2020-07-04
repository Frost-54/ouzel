// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_EFFECT_HPP
#define OUZEL_AUDIO_EFFECT_HPP

#include <cstdint>
#include "Node.hpp"

namespace ouzel::audio
{
    class Audio;
    class Mix;

    class Effect: public Node
    {
        friend Mix;
    public:
        Effect(Audio& initAudio,
               std::uintmax_t initProcessorId);
        ~Effect() override;

        Effect(const Effect&) = delete;
        Effect& operator=(const Effect&) = delete;
        Effect(Effect&&) = delete;
        Effect& operator=(Effect&&) = delete;

        auto getProcessorId() const noexcept { return processorId; }

        auto isEnabled() const noexcept { return enabled; }
        void setEnabled(bool newEnabled);

    protected:
        Audio& audio;
        std::uintmax_t processorId = 0;
        Mix* mix = nullptr;
        bool enabled = true;
    };
}

#endif // OUZEL_AUDIO_EFFECT_HPP
