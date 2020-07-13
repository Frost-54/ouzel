// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_SETTINGS_HPP
#define OUZEL_AUDIO_SETTINGS_HPP

#include <cstdint>
#include "SampleFormat.hpp"

namespace ouzel::audio
{
    struct Settings final
    {
        bool debugAudio = false;
        std::uint32_t bufferSize = 512;
        std::uint32_t sampleRate = 44100;
        std::uint32_t channels = 0;
        SampleFormat sampleFormat = SampleFormat::float32;
    };
}

#endif // OUZEL_AUDIO_SETTINGS_HPP
