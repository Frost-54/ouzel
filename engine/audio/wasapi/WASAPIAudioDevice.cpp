// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../core/Setup.h"

#if OUZEL_COMPILE_WASAPI

#include "WASAPIAudioDevice.hpp"
#include "../../core/Engine.hpp"

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

namespace ouzel
{
    namespace audio
    {
        namespace wasapi
        {
            namespace
            {
                class NotificationClient final: public IMMNotificationClient
                {
                    LONG refCount = 1;

                public:
                    NotificationClient() = default;
                    NotificationClient(const NotificationClient&) = delete;
                    NotificationClient(NotificationClient&&) = delete;
                    NotificationClient& operator=(const NotificationClient&) = delete;
                    NotificationClient& operator=(NotificationClient&&) = delete;

                    ULONG STDMETHODCALLTYPE AddRef()
                    {
                        return InterlockedIncrement(&refCount);
                    }

                    ULONG STDMETHODCALLTYPE Release()
                    {
                        const ULONG newRefCount = InterlockedDecrement(&refCount);
                        if (!newRefCount)
                            delete this;

                        return newRefCount;
                    }

                    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID** ppvInterface)
                    {
                        if (riid == IID_IUnknown ||
                            riid == __uuidof(IMMNotificationClient))
                        {
                            AddRef();
                            *ppvInterface = this;
                        }
                        else
                        {
                            *ppvInterface = nullptr;
                            return E_NOINTERFACE;
                        }
                        return S_OK;
                    }

                    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
                    {
                        return S_OK;
                    };

                    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
                    {
                        return S_OK;
                    }

                    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
                    {
                        return S_OK;
                    }

                    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDeviceId)
                    {
                        if (role == eConsole)
                        {
                            if (flow == eRender)
                            {
                                // TODO: implement
                            }
                            else if (flow == eCapture)
                            {
                                // TODO: implement
                            }
                        }

                        return S_OK;
                    }

                    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
                    {
                        return S_OK;
                    }
                };

                class ErrorCategory final: public std::error_category
                {
                public:
                    const char* name() const noexcept final
                    {
                        return "DirectSound";
                    }

                    std::string message(int condition) const final
                    {
                        switch (condition)
                        {
                            case REGDB_E_CLASSNOTREG: return "REGDB_E_CLASSNOTREG";
                            case CLASS_E_NOAGGREGATION: return "CLASS_E_NOAGGREGATION";
                            case E_POINTER: return "E_POINTER";
                            case E_INVALIDARG: return "E_INVALIDARG";
                            case __HRESULT_FROM_WIN32(ERROR_NOT_FOUND) /* E_NOTFOUND */: return "E_NOTFOUND";
                            case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
                            case E_NOINTERFACE: return "E_NOINTERFACE";
                            case AUDCLNT_E_NOT_INITIALIZED: return "AUDCLNT_E_NOT_INITIALIZED";
                            case AUDCLNT_E_ALREADY_INITIALIZED: return "AUDCLNT_E_ALREADY_INITIALIZED";
                            case AUDCLNT_E_WRONG_ENDPOINT_TYPE: return "AUDCLNT_E_WRONG_ENDPOINT_TYPE";
                            case AUDCLNT_E_DEVICE_INVALIDATED: return "AUDCLNT_E_DEVICE_INVALIDATED";
                            case AUDCLNT_E_NOT_STOPPED: return "AUDCLNT_E_NOT_STOPPED";
                            case AUDCLNT_E_BUFFER_TOO_LARGE: return "AUDCLNT_E_BUFFER_TOO_LARGE";
                            case AUDCLNT_E_OUT_OF_ORDER: return "AUDCLNT_E_OUT_OF_ORDER";
                            case AUDCLNT_E_UNSUPPORTED_FORMAT: return "AUDCLNT_E_UNSUPPORTED_FORMAT";
                            case AUDCLNT_E_INVALID_SIZE: return "AUDCLNT_E_INVALID_SIZE";
                            case AUDCLNT_E_DEVICE_IN_USE: return "AUDCLNT_E_DEVICE_IN_USE";
                            case AUDCLNT_E_BUFFER_OPERATION_PENDING: return "AUDCLNT_E_BUFFER_OPERATION_PENDING";
                            case AUDCLNT_E_THREAD_NOT_REGISTERED: return "AUDCLNT_E_THREAD_NOT_REGISTERED";
                            case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED: return "AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED";
                            case AUDCLNT_E_ENDPOINT_CREATE_FAILED: return "AUDCLNT_E_ENDPOINT_CREATE_FAILED";
                            case AUDCLNT_E_SERVICE_NOT_RUNNING: return "AUDCLNT_E_SERVICE_NOT_RUNNING";
                            case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED: return "AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED";
                            case AUDCLNT_E_EXCLUSIVE_MODE_ONLY: return "AUDCLNT_E_EXCLUSIVE_MODE_ONLY";
                            case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL: return "AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL";
                            case AUDCLNT_E_EVENTHANDLE_NOT_SET: return "AUDCLNT_E_EVENTHANDLE_NOT_SET";
                            case AUDCLNT_E_INCORRECT_BUFFER_SIZE: return "AUDCLNT_E_INCORRECT_BUFFER_SIZE";
                            case AUDCLNT_E_BUFFER_SIZE_ERROR: return "AUDCLNT_E_BUFFER_SIZE_ERROR";
                            case AUDCLNT_E_CPUUSAGE_EXCEEDED: return "AUDCLNT_E_CPUUSAGE_EXCEEDED";
                            case AUDCLNT_E_BUFFER_ERROR: return "AUDCLNT_E_BUFFER_ERROR";
                            case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED: return "AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED";
                            case AUDCLNT_E_INVALID_DEVICE_PERIOD: return "AUDCLNT_E_INVALID_DEVICE_PERIOD";
                            case AUDCLNT_E_INVALID_STREAM_FLAG: return "AUDCLNT_E_INVALID_STREAM_FLAG";
                            case AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE: return "AUDCLNT_E_ENDPOINT_OFFLOAD_NOT_CAPABLE";
                            case AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES: return "AUDCLNT_E_OUT_OF_OFFLOAD_RESOURCES";
                            case AUDCLNT_E_OFFLOAD_MODE_ONLY: return "AUDCLNT_E_OFFLOAD_MODE_ONLY";
                            case AUDCLNT_E_NONOFFLOAD_MODE_ONLY: return "AUDCLNT_E_NONOFFLOAD_MODE_ONLY";
                            case AUDCLNT_E_RESOURCES_INVALIDATED: return "AUDCLNT_E_RESOURCES_INVALIDATED";
                            case AUDCLNT_E_RAW_MODE_UNSUPPORTED: return "AUDCLNT_E_RAW_MODE_UNSUPPORTED";
                            case AUDCLNT_S_BUFFER_EMPTY: return "AUDCLNT_S_BUFFER_EMPTY";
                            case AUDCLNT_S_THREAD_ALREADY_REGISTERED: return "AUDCLNT_S_THREAD_ALREADY_REGISTERED";
                            case AUDCLNT_S_POSITION_STALLED: return "AUDCLNT_S_POSITION_STALLED";
                            default: return "Unknown error (" + std::to_string(condition) + ")";
                        }
                    }
                };

                const ErrorCategory errorCategory{};
            }

            AudioDevice::AudioDevice(std::uint32_t initBufferSize,
                                     std::uint32_t initSampleRate,
                                     std::uint32_t initChannels,
                                     const std::function<void(std::uint32_t frames,
                                                              std::uint32_t channels,
                                                              std::uint32_t sampleRate,
                                                              std::vector<float>& samples)>& initDataGetter):
                audio::AudioDevice(Driver::wasapi, initBufferSize, initSampleRate, initChannels, initDataGetter)
            {
                HRESULT hr;

                LPVOID enumeratorPointer;
                if (FAILED(hr = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, &enumeratorPointer)))
                    throw std::system_error(hr, errorCategory, "Failed to create device enumerator");

                enumerator = static_cast<IMMDeviceEnumerator*>(enumeratorPointer);

                IMMDevice* devicePointer;
                if (FAILED(hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &devicePointer)))
                    throw std::system_error(hr, errorCategory, "Failed to get audio endpoint");

                device = devicePointer;

                notificationClient = new NotificationClient();

                if (FAILED(hr = enumerator->RegisterEndpointNotificationCallback(notificationClient.get())))
                    throw std::system_error(hr, errorCategory, "Failed to get audio endpoint");

                void* audioClientPointer;
                if (FAILED(hr = device->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, &audioClientPointer)))
                    throw std::system_error(hr, errorCategory, "Failed to activate audio device");

                audioClient = static_cast<IAudioClient*>(audioClientPointer);

                WAVEFORMATEX* audioClientWaveFormat;

                if (FAILED(hr = audioClient->GetMixFormat(&audioClientWaveFormat)))
                    throw std::system_error(hr, errorCategory, "Failed to get audio mix format");

                WAVEFORMATEX waveFormat;
                waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
                waveFormat.nChannels = static_cast<WORD>(channels);
                waveFormat.nSamplesPerSec = sampleRate;
                waveFormat.wBitsPerSample = sizeof(float) * 8;
                waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
                waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
                waveFormat.cbSize = 0;

                sampleFormat = SampleFormat::float32;
                sampleSize = sizeof(float);

                const DWORD streamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
                    (waveFormat.nSamplesPerSec != audioClientWaveFormat->nSamplesPerSec ? AUDCLNT_STREAMFLAGS_RATEADJUST : 0);

                CoTaskMemFree(audioClientWaveFormat);

                constexpr std::uint64_t timesPerSecond = 10'000'000U;
                auto bufferPeriod = static_cast<REFERENCE_TIME>(512U * timesPerSecond / waveFormat.nSamplesPerSec);

                WAVEFORMATEX* closesMatch;
                if (!FAILED(audioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED,
                                                           &waveFormat,
                                                           &closesMatch)))
                {
                    // TODO: implement
                }

                CoTaskMemFree(closesMatch);

                if (FAILED(hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                        streamFlags,
                                                        bufferPeriod,
                                                        0,
                                                        &waveFormat,
                                                        nullptr)))
                {
                    waveFormat.wFormatTag = WAVE_FORMAT_PCM;
                    waveFormat.wBitsPerSample = sizeof(std::int16_t) * 8;
                    waveFormat.nBlockAlign = waveFormat.nChannels * (waveFormat.wBitsPerSample / 8);
                    waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

                    if (FAILED(hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                                            streamFlags,
															bufferPeriod,
                                                            0,
                                                            &waveFormat,
                                                            nullptr)))
                        throw std::system_error(hr, errorCategory, "Failed to initialize audio client");

                    sampleFormat = SampleFormat::signedInt16;
                    sampleSize = sizeof(std::int16_t);
                }

                // init output device
                if (FAILED(hr = audioClient->GetBufferSize(&bufferFrameCount)))
                    throw std::system_error(hr, errorCategory, "Failed to get audio buffer size");
                bufferSize = bufferFrameCount * channels;

                void* renderClientPointer;
                if (FAILED(hr = audioClient->GetService(IID_IAudioRenderClient, &renderClientPointer)))
                    throw std::system_error(GetLastError(), std::system_category(), "Failed to get render client service");

                renderClient = static_cast<IAudioRenderClient*>(renderClientPointer);

                notifyEvent = CreateEvent(nullptr, false, false, nullptr);
                if (!notifyEvent)
                    throw std::system_error(GetLastError(), std::system_category(), "Failed to create event");

                if (FAILED(hr = audioClient->SetEventHandle(notifyEvent)))
                    throw std::system_error(hr, errorCategory, "Failed to set event handle");
            }

            AudioDevice::~AudioDevice()
            {
                running = false;
                if (notifyEvent) SetEvent(notifyEvent);

                if (audioThread.isJoinable()) audioThread.join();

                if (notifyEvent) CloseHandle(notifyEvent);

                if (audioClient && started) audioClient->Stop();
            }

            void AudioDevice::start()
            {
                HRESULT hr;
                if (FAILED(hr = audioClient->Start()))
                    throw std::system_error(hr, errorCategory, "Failed to start audio");

                started = true;
                running = true;
                audioThread = Thread(&AudioDevice::run, this);
            }

            void AudioDevice::stop()
            {
                running = false;
                if (audioThread.isJoinable()) audioThread.join();

                if (started)
                {
                    HRESULT hr;
                    if (FAILED(hr = audioClient->Stop()))
                        throw std::system_error(hr, errorCategory, "Failed to stop audio");
                    
                    started = false;
                }
            }

            void AudioDevice::run()
            {
                Thread::setCurrentThreadName("Audio");

                while (running)
                {
                    try
                    {
                        DWORD result;
                        if ((result = WaitForSingleObject(notifyEvent, INFINITE)) == WAIT_FAILED)
                            throw std::system_error(GetLastError(), std::system_category(), "Failed to wait for event");

                        if (result == WAIT_OBJECT_0)
                        {
                            if (!running) break;

                            HRESULT hr;
                            UINT32 bufferPadding;
                            if (FAILED(hr = audioClient->GetCurrentPadding(&bufferPadding)))
                                throw std::system_error(hr, errorCategory, "Failed to get buffer padding");

                            const UINT32 frameCount = bufferFrameCount - bufferPadding;
                            if (frameCount != 0)
                            {
                                BYTE* renderBuffer;
                                if (FAILED(hr = renderClient->GetBuffer(frameCount, &renderBuffer)))
                                    throw std::system_error(hr, errorCategory, "Failed to get buffer");

                                getData(frameCount, data);

                                std::copy(data.begin(), data.end(), renderBuffer);

                                if (FAILED(hr = renderClient->ReleaseBuffer(frameCount, 0)))
                                    throw std::system_error(hr, errorCategory, "Failed to release buffer");
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        engine->log(Log::Level::error) << e.what();
                    }
                }
            }
        } // namespace wasapi
    } // namespace audio
} // namespace ouzel

#endif
