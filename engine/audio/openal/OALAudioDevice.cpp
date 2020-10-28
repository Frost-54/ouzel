// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../core/Setup.h"

#if OUZEL_COMPILE_OPENAL

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif
#if TARGET_OS_IOS || TARGET_OS_TV
#  include <objc/message.h>
#  include <objc/NSObjCRuntime.h>
extern "C" id const AVAudioSessionCategoryAmbient;
#endif

#include "OALAudioDevice.hpp"
#include "OALErrorCategory.hpp"
#include "ALCErrorCategory.hpp"
#include "../../core/Engine.hpp"
#include "../../utils/Log.hpp"

#ifndef AL_FORMAT_MONO_FLOAT32
#  define AL_FORMAT_MONO_FLOAT32 0x10010
#endif

#ifndef AL_FORMAT_STEREO_FLOAT32
#  define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

namespace ouzel::audio::openal
{
    namespace
    {
        const ErrorCategory openALErrorCategory{};
        const alc::ErrorCategory alcErrorCategory{};
    }

    AudioDevice::AudioDevice(const Settings& settings,
                             const std::function<void(std::uint32_t frames,
                                                      std::uint32_t channels,
                                                      std::uint32_t sampleRate,
                                                      std::vector<float>& samples)>& initDataGetter):
        audio::AudioDevice(Driver::openAL, settings, initDataGetter)
    {
#if TARGET_OS_IOS || TARGET_OS_TV
        id audioSession = reinterpret_cast<id (*)(Class, SEL)>(&objc_msgSend)(objc_getClass("AVAudioSession"), sel_getUid("sharedInstance")); // [AVAudioSession sharedInstance]
        if (!reinterpret_cast<BOOL (*)(id, SEL, id, id)>(&objc_msgSend)(audioSession, sel_getUid("setCategory:error:"), AVAudioSessionCategoryAmbient, nil)) // [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil]
            throw std::runtime_error("Failed to set audio session category");

        id currentRoute = reinterpret_cast<id (*)(id, SEL)>(&objc_msgSend)(audioSession, sel_getUid("currentRoute")); // [audioSession currentRoute]
        id outputs = reinterpret_cast<id (*)(id, SEL)>(&objc_msgSend)(currentRoute, sel_getUid("outputs")); // [currentRoute outputs]
        const auto count = reinterpret_cast<NSUInteger (*)(id, SEL)>(&objc_msgSend)(outputs, sel_getUid("count")); // [outputs count]

        NSUInteger maxChannelCount = 0;
        for (NSUInteger outputIndex = 0; outputIndex < count; ++outputIndex)
        {
            id output = reinterpret_cast<id (*)(id, SEL, NSUInteger)>(&objc_msgSend)(outputs, sel_getUid("objectAtIndex:"), outputIndex); // [outputs objectAtIndex:outputIndex]
            id channels = reinterpret_cast<id (*)(id, SEL)>(&objc_msgSend)(output, sel_getUid("channels")); // [output channels]
            const auto channelCount = reinterpret_cast<NSUInteger (*)(id, SEL)>(&objc_msgSend)(channels, sel_getUid("count")); // [channels count]
            if (channelCount > maxChannelCount)
                maxChannelCount = channelCount;
        }

        if (channels > maxChannelCount)
            channels = static_cast<std::uint32_t>(maxChannelCount);
#endif

        const auto deviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);

        logger.log(Log::Level::info) << "Using " << deviceName << " for audio";

        device = alcOpenDevice(deviceName);

        if (!device)
            throw std::runtime_error("Failed to open ALC device");

        if (const auto error = alcGetError(device); error != ALC_NO_ERROR)
            throw std::system_error(error, alcErrorCategory, "Failed to create ALC device");

        context = alcCreateContext(device, nullptr);

        if (const auto error = alcGetError(device); error != ALC_NO_ERROR)
            throw std::system_error(error, alcErrorCategory, "Failed to create ALC context");

        if (!context)
            throw std::runtime_error("Failed to create ALC context");

        if (!alcMakeContextCurrent(context))
            throw std::runtime_error("Failed to make ALC context current");

        if (const auto error = alcGetError(device); error != ALC_NO_ERROR)
            throw std::system_error(error, alcErrorCategory, "Failed to make ALC context current");

        const auto audioRenderer = alGetString(AL_RENDERER);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            logger.log(Log::Level::warning) << "Failed to get OpenAL renderer, error: " + std::to_string(error);
        else if (!audioRenderer)
            logger.log(Log::Level::warning) << "Failed to get OpenAL renderer";
        else
            logger.log(Log::Level::info) << "Using " << audioRenderer << " audio renderer";

        std::vector<std::string> extensions;
        const auto extensionsPtr = alGetString(AL_EXTENSIONS);

        if (const auto error = alGetError(); error != AL_NO_ERROR || !extensionsPtr)
            logger.log(Log::Level::warning) << "Failed to get OpenGL extensions";
        else
            extensions = explodeString(std::string(extensionsPtr), ' ');

        logger.log(Log::Level::all) << "Supported OpenAL extensions: " << extensions;

        auto float32Supported = false;
        for (const std::string& extension : extensions)
        {
            if (extension == "AL_EXT_float32")
                float32Supported = true;
        }

#if !defined(__EMSCRIPTEN__)
        format40 = alGetEnumValue("AL_FORMAT_QUAD16");
        format51 = alGetEnumValue("AL_FORMAT_51CHN16");
        format61 = alGetEnumValue("AL_FORMAT_61CHN16");
        format71 = alGetEnumValue("AL_FORMAT_71CHN16");

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            logger.log(Log::Level::warning) << "Failed to get OpenAL enum values";
#endif

        alGenSources(1, &sourceId);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to create OpenAL source");

        alGenBuffers(2, bufferIds);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to create OpenAL buffers");

        switch (channels)
        {
            case 1:
            {
                if (float32Supported)
                {
                    format = AL_FORMAT_MONO_FLOAT32;
                    sampleFormat = SampleFormat::float32;
                    sampleSize = sizeof(float);
                }
                else
                {
                    format = AL_FORMAT_MONO16;
                    sampleFormat = SampleFormat::signedInt16;
                    sampleSize = sizeof(std::int16_t);
                }
                break;
            }
            case 2:
            {
                if (float32Supported)
                {
                    format = AL_FORMAT_STEREO_FLOAT32;
                    sampleFormat = SampleFormat::float32;
                    sampleSize = sizeof(float);
                }
                else
                {
                    format = AL_FORMAT_STEREO16;
                    sampleFormat = SampleFormat::signedInt16;
                    sampleSize = sizeof(std::int16_t);
                }
                break;
            }
            case 4:
            {
                format = format40;
                sampleFormat = SampleFormat::signedInt16;
                sampleSize = sizeof(std::int16_t);
                break;
            }
            case 6:
            {
                format = format51;
                sampleFormat = SampleFormat::signedInt16;
                sampleSize = sizeof(std::int16_t);
                break;
            }
            case 7:
            {
                format = format61;
                sampleFormat = SampleFormat::signedInt16;
                sampleSize = sizeof(std::int16_t);
                break;
            }
            case 8:
            {
                format = format71;
                sampleFormat = SampleFormat::signedInt16;
                sampleSize = sizeof(std::int16_t);
                break;
            }
            default:
                throw std::runtime_error("Invalid channel count");
        }
    }

    AudioDevice::~AudioDevice()
    {
#if !defined(__EMSCRIPTEN__)
        running = false;
        if (audioThread.isJoinable()) audioThread.join();
#endif

        if (sourceId)
        {
            alSourceStop(sourceId);
            alSourcei(sourceId, AL_BUFFER, 0);
            alDeleteSources(1, &sourceId);
            alGetError();
        }

        for (const auto bufferId : bufferIds)
        {
            if (bufferId)
            {
                alDeleteBuffers(1, &bufferId);
                alGetError();
            }
        }

        if (context)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(context);
        }

        if (device)
            alcCloseDevice(device);
    }

    void AudioDevice::start()
    {
        getData(bufferSize / (channels * sampleSize), data);

        alBufferData(bufferIds[0], format,
                     data.data(),
                     static_cast<ALsizei>(data.size()),
                     static_cast<ALsizei>(sampleRate));

        getData(bufferSize / (channels * sampleSize), data);

        alBufferData(bufferIds[1], format,
                     data.data(),
                     static_cast<ALsizei>(data.size()),
                     static_cast<ALsizei>(sampleRate));

        nextBuffer = 0;

        alSourceQueueBuffers(sourceId, 2, bufferIds);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to queue OpenAL buffers");

        alSourcePlay(sourceId);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to play OpenAL source");

#if !defined(__EMSCRIPTEN__)
        running = true;
        audioThread = thread::Thread(&AudioDevice::run, this);
#endif
    }

    void AudioDevice::stop()
    {
#if !defined(__EMSCRIPTEN__)
        running = false;
        if (audioThread.isJoinable()) audioThread.join();
#endif

        alSourceStop(sourceId);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to stop OpenAL source");
    }

    void AudioDevice::process()
    {
        if (!alcMakeContextCurrent(context))
            throw std::runtime_error("Failed to make ALC context current");

        if (const auto error = alcGetError(device); error != ALC_NO_ERROR)
            throw std::system_error(error, alcErrorCategory, "Failed to make ALC context current");

        ALint buffersProcessed;
        alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &buffersProcessed);

        if (const auto error = alGetError(); error != AL_NO_ERROR)
            throw std::system_error(error, openALErrorCategory, "Failed to get processed buffer count");

        // requeue all processed buffers
        for (; buffersProcessed > 0; --buffersProcessed)
        {
            alSourceUnqueueBuffers(sourceId, 1, &bufferIds[nextBuffer]);

            if (const auto error = alGetError(); error != AL_NO_ERROR)
                throw std::system_error(error, openALErrorCategory, "Failed to unqueue OpenAL buffer");

            getData(bufferSize / (channels * sampleSize), data);

            alBufferData(bufferIds[nextBuffer], format,
                         data.data(),
                         static_cast<ALsizei>(data.size()),
                         static_cast<ALsizei>(sampleRate));

            alSourceQueueBuffers(sourceId, 1, &bufferIds[nextBuffer]);

            if (const auto error = alGetError(); error != AL_NO_ERROR)
                throw std::system_error(error, openALErrorCategory, "Failed to queue OpenAL buffer");

            ALint state;
            alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING)
            {
                alSourcePlay(sourceId);

                if (const auto error = alGetError(); error != AL_NO_ERROR)
                    throw std::system_error(error, openALErrorCategory, "Failed to play OpenAL source");
            }

            // swap the buffer
            nextBuffer = (nextBuffer + 1) % 2;
        }
    }

    void AudioDevice::run()
    {
        thread::setCurrentThreadName("Audio");

#if !defined(__EMSCRIPTEN__)
        while (running)
        {
            try
            {
                process();
            }
            catch (const std::exception& e)
            {
                ouzel::logger.log(ouzel::Log::Level::error) << e.what();
            }
        }
#endif
    }
}
#endif
