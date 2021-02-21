// Copyright 2015-2021 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_OPENGLVIEW_H
#define OUZEL_GRAPHICS_OPENGLVIEW_H

#include "../../../core/Setup.h"

#if OUZEL_COMPILE_OPENGL

#import <AppKit/NSOpenGL.h>
#import "../../../core/macos/ViewMacOS.h"

@interface OpenGLView: View

- (void)setOpenGLContext:(NSOpenGLContext*)context;
@end

#endif

#endif // OUZEL_GRAPHICS_OPENGLVIEW_H
