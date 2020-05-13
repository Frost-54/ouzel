// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <stdexcept>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "NativeWindowEm.hpp"

namespace ouzel
{
    namespace
    {
        EM_BOOL emResizeCallback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
        {
            if (eventType == EMSCRIPTEN_EVENT_RESIZE)
            {
                auto nativeWindowEm = static_cast<NativeWindowEm*>(userData);
                nativeWindowEm->handleResize();
                return true;
            }

            return false;
        }

        EM_BOOL emFullscreenCallback(int eventType, const void*, void* userData)
        {
            if (eventType == EMSCRIPTEN_EVENT_CANVASRESIZED)
            {
                auto nativeWindowEm = static_cast<NativeWindowEm*>(userData);
                nativeWindowEm->handleResize();
                return true;
            }

            return false;
        }
    }

    NativeWindowEm::NativeWindowEm(const std::function<void(const Event&)>& initCallback,
                                   const Size2U& newSize,
                                   bool newFullscreen,
                                   const std::string& newTitle,
                                   bool newHighDpi):
        NativeWindow(initCallback,
                     newSize,
                     true,
                     newFullscreen,
                     true,
                     newTitle,
                     newHighDpi)
    {
        emscripten_set_resize_callback(nullptr, this, true, emResizeCallback);

        if (size.v[0] == 0 || size.v[1] == 0)
        {
            int width, height, isFullscreen;
            emscripten_get_canvas_size(&width, &height, &isFullscreen);

            if (size.v[0] == 0) size.v[0] = static_cast<std::uint32_t>(width);
            if (size.v[1] == 0) size.v[1] = static_cast<std::uint32_t>(height);
            fullscreen = static_cast<bool>(isFullscreen);
        }
        else
            emscripten_set_canvas_size(static_cast<int>(size.v[0]),
                                       static_cast<int>(size.v[1]));

        if (fullscreen)
        {
            EmscriptenFullscreenStrategy s;
            s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
            s.canvasResolutionScaleMode = highDpi ? EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF : EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
            s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            s.canvasResizedCallback = emFullscreenCallback;
            s.canvasResizedCallbackUserData = this;

            emscripten_request_fullscreen_strategy(nullptr, EM_TRUE, &s);
        }

        resolution = size;
    }

    void NativeWindowEm::executeCommand(const Command& command)
    {
        switch (command.type)
        {
            case Command::Type::changeSize:
                setSize(command.size);
                break;
            case Command::Type::changeFullscreen:
                setFullscreen(command.fullscreen);
                break;
            case Command::Type::close:
                break;
            case Command::Type::setTitle:
                break;
            case Command::Type::bringToFront:
                break;
            case Command::Type::show:
                break;
            case Command::Type::hide:
                break;
            case Command::Type::minimize:
                break;
            case Command::Type::maximize:
                break;
            case Command::Type::restore:
                break;
            default:
                throw std::runtime_error("Invalid command");
        }
    }

    void NativeWindowEm::setSize(const Size2U& newSize)
    {
        size = newSize;

        emscripten_set_canvas_size(static_cast<int>(newSize.v[0]),
                                   static_cast<int>(newSize.v[1]));
    }

    void NativeWindowEm::setFullscreen(bool newFullscreen)
    {
        fullscreen = newFullscreen;

        if (newFullscreen)
        {
            EmscriptenFullscreenStrategy s;
            s.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
            s.canvasResolutionScaleMode = highDpi ? EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF : EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
            s.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
            s.canvasResizedCallback = emFullscreenCallback;
            s.canvasResizedCallbackUserData = this;

            emscripten_request_fullscreen_strategy(nullptr, EM_TRUE, &s);
        }
        else
            emscripten_exit_fullscreen();
    }

    void NativeWindowEm::handleResize()
    {
        int width, height, isFullscreen;
        emscripten_get_canvas_size(&width, &height, &isFullscreen);

        Size2U newSize(static_cast<std::uint32_t>(width),
                       static_cast<std::uint32_t>(height));

        size = newSize;
        resolution = size;

        auto oldFullscreen = fullscreen;
        fullscreen = (isFullscreen != 0);

        Event sizeChangeEvent(Event::Type::sizeChange);
        sizeChangeEvent.size = size;
        sendEvent(sizeChangeEvent);

        Event resolutionChangeEvent(Event::Type::resolutionChange);
        resolutionChangeEvent.size = resolution;
        sendEvent(resolutionChangeEvent);

        if (fullscreen != oldFullscreen)
        {
            Event fullscreenChangeEvent(Event::Type::fullscreenChange);
            fullscreenChangeEvent.fullscreen = fullscreen;
            sendEvent(fullscreenChangeEvent);
        }
    }
}
