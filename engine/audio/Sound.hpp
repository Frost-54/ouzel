// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_SOUND_HPP
#define OUZEL_AUDIO_SOUND_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ouzel::audio
{
    class Audio;

    class Sound
    {
        friend Audio;
    public:
        enum class Format
        {
            pcm,
            vorbis
        };

        Sound(Audio& initAudio, std::uintptr_t initSourceId, Format initFormat);
        virtual ~Sound();

        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;

        Sound(Sound&&) = delete;
        Sound& operator=(Sound&&) = delete;

        auto getSourceId() const noexcept { return sourceId; }
        auto getFormat() const noexcept { return format; }

    protected:
        Audio& audio;
        std::uintptr_t sourceId = 0;
        Format format;
    };
}

#endif // OUZEL_AUDIO_SOUND_HPP
