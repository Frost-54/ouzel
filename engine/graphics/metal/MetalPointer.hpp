// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_METALPOINTER_HPP
#define OUZEL_GRAPHICS_METALPOINTER_HPP

#include "../../core/Setup.h"

#if OUZEL_COMPILE_METAL

#ifndef __OBJC__
# include <objc/message.h>
# include <objc/objc.h>
#endif

namespace ouzel
{
    namespace graphics
    {
        namespace metal
        {
            template <class T>
            class Pointer final
            {
            public:
                Pointer() noexcept = default;

                Pointer(T a) noexcept: p(a) {}
                Pointer& operator=(T a) noexcept
                {
                    if (p)
#ifdef __OBJC__
                        [p release];
#else
                        objc_msgSend(p, sel_getUid("release"));
#endif

                    p = a;
                    return *this;
                }

                Pointer(const Pointer&) = delete;
                Pointer& operator=(const Pointer&) = delete;

                Pointer(Pointer&& other) noexcept: p(other.p)
                {
                    other.p = nil;
                }

                Pointer& operator=(Pointer&& other) noexcept
                {
                    if (this == &other) return *this;
                    if (p)
#ifdef __OBJC__
                        [p release];
#else
                        objc_msgSend(p, sel_getUid("release"));
#endif
                    p = other.p;
                    other.p = nil;
                    return *this;
                }

                ~Pointer()
                {
                    if (p)
#ifdef __OBJC__
                        [p release];
#else
                        objc_msgSend(p, sel_getUid("release"));
#endif
                }

                T get() const noexcept
                {
                    return p;
                }

                explicit operator bool() const noexcept
                {
                    return p != nil;
                }

            private:
                T p = nil;
            };
        } // namespace metal
    } // namespace graphics
} // namespace ouzel

#endif

#endif // OUZEL_GRAPHICS_METALPOINTER_HPP
