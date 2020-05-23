// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../Setup.h"

#include <stdexcept>
#if OUZEL_COMPILE_OPENGL && OUZEL_SUPPORTS_X11
#  include <GL/glx.h>
#endif

#include "NativeWindowLinux.hpp"
#include "EngineLinux.hpp"
#include "../../graphics/RenderDevice.hpp"

#if OUZEL_SUPPORTS_X11
namespace
{
    constexpr long NET_WM_STATE_REMOVE = 0L;
    constexpr long NET_WM_STATE_ADD = 1L;
    constexpr long NET_WM_STATE_TOGGLE = 2L;
}
#endif

namespace ouzel
{
    NativeWindowLinux::NativeWindowLinux(const std::function<void(const Event&)>& initCallback,
                                         const Size2U& newSize,
                                         bool newResizable,
                                         bool newFullscreen,
                                         bool newExclusiveFullscreen,
                                         const std::string& newTitle,
                                         graphics::Driver graphicsDriver,
                                         bool depth):
        NativeWindow(initCallback,
                     newSize,
                     newResizable,
                     newFullscreen,
                     newExclusiveFullscreen,
                     newTitle,
                     true)
    {
#if OUZEL_SUPPORTS_X11
        auto engineLinux = static_cast<EngineLinux*>(engine);
        display = engineLinux->getDisplay();

        auto screen = XDefaultScreenOfDisplay(display);
        screenNumber = XScreenNumberOfScreen(screen);

        if (size.v[0] <= 0.0F) size.v[0] = static_cast<std::uint32_t>(XWidthOfScreen(screen) * 0.8F);
        if (size.v[1] <= 0.0F) size.v[1] = static_cast<std::uint32_t>(XHeightOfScreen(screen) * 0.8F);

        resolution = size;

        const int x = XWidthOfScreen(screen) / 2 - static_cast<int>(size.v[0] / 2);
        const int y = XHeightOfScreen(screen) / 2 - static_cast<int>(size.v[1] / 2);

        switch (graphicsDriver)
        {
            case graphics::Driver::empty:
            {
                XSetWindowAttributes swa;
                swa.background_pixel = XWhitePixel(display, screenNumber);
                swa.border_pixel = 0;
                swa.event_mask = FocusChangeMask | KeyPressMask | KeyRelease | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

                window = XCreateWindow(display, RootWindow(display, screenNumber), x, y,
                                       static_cast<unsigned int>(size.v[0]), static_cast<unsigned int>(size.v[1]),
                                       0, DefaultDepth(display, screenNumber), InputOutput, DefaultVisual(display, screenNumber),
                                       CWBorderPixel | CWBackPixel | CWEventMask, &swa);

                break;
            }
#  if OUZEL_COMPILE_OPENGL
            case graphics::Driver::openGL:
            {
                // find an OpenGL-capable RGB visual
                static int doubleBuffer[] = {
                    GLX_RGBA,
                    GLX_RED_SIZE, 8,
                    GLX_GREEN_SIZE, 8,
                    GLX_BLUE_SIZE, 8,
                    GLX_DEPTH_SIZE, depth ? 24 : 0,
                    GLX_DOUBLEBUFFER,
                    None
                };

                visualInfo = glXChooseVisual(display, screenNumber, doubleBuffer);
                if (!visualInfo)
                    throw std::runtime_error("Failed to choose visual");

                if (visualInfo->c_class != TrueColor)
                    throw std::runtime_error("TrueColor visual required for this program");

                // create an X colormap since probably not using default visual
                Colormap cmap = XCreateColormap(display, RootWindow(display, visualInfo->screen), visualInfo->visual, AllocNone);
                XSetWindowAttributes swa;
                swa.colormap = cmap;
                swa.border_pixel = 0;
                swa.event_mask = FocusChangeMask | KeyPressMask | KeyRelease | ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

                window = XCreateWindow(display, RootWindow(display, visualInfo->screen), x, y,
                                       static_cast<unsigned int>(size.v[0]), static_cast<unsigned int>(size.v[1]),
                                       0, visualInfo->depth, InputOutput, visualInfo->visual,
                                       CWBorderPixel | CWColormap | CWEventMask, &swa);
                break;
            }
#  endif
            default:
                throw std::runtime_error("Unsupported render driver");
        }

        XSetStandardProperties(display, window, title.c_str(), title.c_str(), None, nullptr, 0, nullptr);

        if (!resizable)
        {
            XSizeHints sizeHints;
            sizeHints.flags = PPosition | PMinSize | PMaxSize;
            sizeHints.x = x;
            sizeHints.y = y;
            sizeHints.min_width = static_cast<int>(size.v[0]);
            sizeHints.max_width = static_cast<int>(size.v[0]);
            sizeHints.min_height = static_cast<int>(size.v[1]);
            sizeHints.max_height = static_cast<int>(size.v[1]);
            XSetWMNormalHints(display, window, &sizeHints);
        }
        else
        {
            XSizeHints sizeHints;
            sizeHints.flags = PPosition;
            sizeHints.x = x;
            sizeHints.y = y;
            XSetWMNormalHints(display, window, &sizeHints);
        }

        // request the X window to be displayed on the screen
        XMapWindow(display, window);

        protocolsAtom = XInternAtom(display, "WM_PROTOCOLS", False);
        deleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(display, window, &deleteAtom, 1);
        stateAtom = XInternAtom(display, "_NET_WM_STATE", False);
        stateFullscreenAtom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
        activateWindowAtom = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);

        if (fullscreen)
        {
            if (!stateAtom)
                throw std::runtime_error("State atom is null");

            if (!stateFullscreenAtom)
                throw std::runtime_error("Fullscreen state atom is null");

            XEvent event;
            event.type = ClientMessage;
            event.xclient.window = window;
            event.xclient.message_type = stateAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = NET_WM_STATE_ADD;
            event.xclient.data.l[1] = stateFullscreenAtom;
            event.xclient.data.l[2] = 0; // no second property to toggle
            event.xclient.data.l[3] = 1; // source indication: application
            event.xclient.data.l[4] = 0; // unused

            if (!XSendEvent(display, DefaultRootWindow(display), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event))
                throw std::runtime_error("Failed to send X11 fullscreen message");
        }
#elif OUZEL_SUPPORTS_DISPMANX
        auto engineLinux = static_cast<EngineLinux*>(engine);
        auto display = engineLinux->getDisplay();

        DISPMANX_MODEINFO_T modeInfo;
        const std::int32_t success = vc_dispmanx_display_get_info(display, &modeInfo);

        if (success < 0)
            throw std::runtime_error("Failed to get display size");

        VC_RECT_T dstRect;
        dstRect.x = 0;
        dstRect.y = 0;
        dstRect.width = modeInfo.width;
        dstRect.height = modeInfo.height;

        VC_RECT_T srcRect;
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.width = modeInfo.width;
        srcRect.height = modeInfo.height;

        DISPMANX_UPDATE_HANDLE_T dispmanUpdate = vc_dispmanx_update_start(0);

        if (dispmanUpdate == DISPMANX_NO_HANDLE)
            throw std::runtime_error("Failed to start display update");

        DISPMANX_ELEMENT_HANDLE_T dispmanElement = vc_dispmanx_element_add(dispmanUpdate, display,
                                                                           0, &dstRect, 0,
                                                                           &srcRect, DISPMANX_PROTECTION_NONE,
                                                                           0, 0, DISPMANX_NO_ROTATE);

        if (dispmanElement == DISPMANX_NO_HANDLE)
            throw std::runtime_error("Failed to add display element");

        window.element = dispmanElement;
        window.width = modeInfo.width;
        window.height = modeInfo.height;
        vc_dispmanx_update_submit_sync(dispmanUpdate);

        size.v[0] = static_cast<std::uint32_t>(modeInfo.width);
        size.v[1] = static_cast<std::uint32_t>(modeInfo.height);
        resolution = size;
#endif
    }

    NativeWindowLinux::~NativeWindowLinux()
    {
#if OUZEL_SUPPORTS_X11
        if (visualInfo)
            XFree(visualInfo);

        if (display && window)
            XDestroyWindow(display, window);
#endif
    }

    void NativeWindowLinux::executeCommand(const Command& command)
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
                close();
                break;
            case Command::Type::setTitle:
                setTitle(command.title);
                break;
            case Command::Type::bringToFront:
                bringToFront();
                break;
            case Command::Type::show:
                show();
                break;
            case Command::Type::hide:
                hide();
                break;
            case Command::Type::minimize:
                minimize();
                break;
            case Command::Type::maximize:
                maximize();
                break;
            case Command::Type::restore:
                restore();
                break;
            default:
                throw std::runtime_error("Invalid command");
        }
    }

    void NativeWindowLinux::close()
    {
#if OUZEL_SUPPORTS_X11
        if (!protocolsAtom || !deleteAtom) return;

        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = protocolsAtom;
        event.xclient.format = 32; // data is set as 32-bit integers
        event.xclient.data.l[0] = deleteAtom;
        event.xclient.data.l[1] = CurrentTime;
        event.xclient.data.l[2] = 0; // unused
        event.xclient.data.l[3] = 0; // unused
        event.xclient.data.l[4] = 0; // unused
        if (!XSendEvent(display, window, False, NoEventMask, &event))
            throw std::runtime_error("Failed to send X11 delete message");
#endif
    }

    void NativeWindowLinux::setSize(const Size2U& newSize)
    {
        size = newSize;

#if OUZEL_SUPPORTS_X11
        XWindowChanges changes;
        changes.width = static_cast<int>(size.v[0]);
        changes.height = static_cast<int>(size.v[1]);
        XConfigureWindow(display, window, CWWidth | CWHeight, &changes);

        if (!resizable)
        {
            XSizeHints sizeHints;
            sizeHints.flags = PMinSize | PMaxSize;
            sizeHints.min_width = static_cast<int>(size.v[0]);
            sizeHints.max_width = static_cast<int>(size.v[0]);
            sizeHints.min_height = static_cast<int>(size.v[1]);
            sizeHints.max_height = static_cast<int>(size.v[1]);
            XSetWMNormalHints(display, window, &sizeHints);
        }

        resolution = size;

        Event resolutionChangeEvent(Event::Type::resolutionChange);
        resolutionChangeEvent.size = resolution;
        sendEvent(resolutionChangeEvent);
#endif
    }

    void NativeWindowLinux::setFullscreen(bool newFullscreen)
    {
#if OUZEL_SUPPORTS_X11
        if (fullscreen != newFullscreen)
        {
            if (!stateAtom)
                throw std::runtime_error("State atom is null");

            if (!stateFullscreenAtom)
                throw std::runtime_error("Fullscreen state atom is null");

            XEvent event;
            event.type = ClientMessage;
            event.xclient.window = window;
            event.xclient.message_type = stateAtom;
            event.xclient.format = 32;
            event.xclient.data.l[0] = newFullscreen ? NET_WM_STATE_ADD : NET_WM_STATE_REMOVE;
            event.xclient.data.l[1] = stateFullscreenAtom;
            event.xclient.data.l[2] = 0; // no second property to toggle
            event.xclient.data.l[3] = 1; // source indication: application
            event.xclient.data.l[4] = 0; // unused

            if (!XSendEvent(display, DefaultRootWindow(display), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event))
                throw std::runtime_error("Failed to send X11 fullscreen message");
        }
#endif

        fullscreen = newFullscreen;
    }

    void NativeWindowLinux::setTitle(const std::string& newTitle)
    {
#if OUZEL_SUPPORTS_X11
        if (title != newTitle) XStoreName(display, window, newTitle.c_str());
#endif

        title = newTitle;
    }

    void NativeWindowLinux::bringToFront()
    {
#if OUZEL_SUPPORTS_X11
        XRaiseWindow(display, window);

        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = activateWindowAtom;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1; // source indication: application
        event.xclient.data.l[1] = CurrentTime;
        event.xclient.data.l[2] = 0;

        if (!XSendEvent(display, DefaultRootWindow(display), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event))
            throw std::runtime_error("Failed to send X11 activate window message");

        XFlush(display);
#endif
    }

    void NativeWindowLinux::show()
    {
#if OUZEL_SUPPORTS_X11
        if (!isMapped())
        {
            XMapRaised(display, window);
            XFlush(display);
        }
#endif
    }

    void NativeWindowLinux::hide()
    {
#if OUZEL_SUPPORTS_X11
        if (isMapped())
        {
            XWithdrawWindow(display, window, screenNumber);
            XFlush(display);
        }
#endif
    }

    void NativeWindowLinux::minimize()
    {
#if OUZEL_SUPPORTS_X11
        XIconifyWindow(display, window, screenNumber);
        XFlush(display);
#endif
    }

    void NativeWindowLinux::maximize()
    {
    }

    void NativeWindowLinux::restore()
    {
#if OUZEL_SUPPORTS_X11
        XRaiseWindow(display, window);

        XEvent event;
        event.type = ClientMessage;
        event.xclient.window = window;
        event.xclient.message_type = activateWindowAtom;
        event.xclient.format = 32;
        event.xclient.data.l[0] = 1; // source indication: application
        event.xclient.data.l[1] = CurrentTime;
        event.xclient.data.l[2] = 0;

        if (!XSendEvent(display, DefaultRootWindow(display), 0, SubstructureNotifyMask | SubstructureRedirectMask, &event))
            throw std::runtime_error("Failed to send X11 activate window message");

        XFlush(display);
#endif
    }

#if OUZEL_SUPPORTS_X11
    void NativeWindowLinux::handleFocusIn()
    {
        Event focusChangeEvent(Event::Type::focusChange);
        focusChangeEvent.focus = true;
        sendEvent(focusChangeEvent);
    }

    void NativeWindowLinux::handleFocusOut()
    {
        Event focusChangeEvent(Event::Type::focusChange);
        focusChangeEvent.focus = false;
        sendEvent(focusChangeEvent);
    }

    void NativeWindowLinux::handleResize(const Size2U& newSize)
    {
        size = newSize;
        resolution = size;

        Event sizeChangeEvent(Event::Type::sizeChange);
        sizeChangeEvent.size = size;
        sendEvent(sizeChangeEvent);

        Event resolutionChangeEvent(Event::Type::resolutionChange);
        resolutionChangeEvent.size = resolution;
        sendEvent(resolutionChangeEvent);
    }

    void NativeWindowLinux::handleMap()
    {
        sendEvent(Event(Event::Type::restore));
    }

    void NativeWindowLinux::handleUnmap()
    {
        sendEvent(Event(Event::Type::minimize));
    }

    bool NativeWindowLinux::isMapped() const
    {
        XWindowAttributes attributes;
        XGetWindowAttributes(display, window, &attributes);
        return attributes.map_state != IsUnmapped;
    }
#endif
}
