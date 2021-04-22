// Ouzel by Elviss Strazdins

#ifndef AUDIO_DRIVER_HPP
#define AUDIO_DRIVER_HPP

namespace ouzel::audio
{
    enum class Driver
    {
        empty,
        openAl,
        xAudio2,
        openSl,
        coreAudio,
        alsa,
        wasapi
    };
}

#endif // AUDIO_DRIVER_HPP
