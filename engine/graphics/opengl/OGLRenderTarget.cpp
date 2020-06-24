// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "../../core/Setup.h"

#if OUZEL_COMPILE_OPENGL

#include <stdexcept>
#include "OGLRenderTarget.hpp"
#include "OGLRenderDevice.hpp"
#include "OGLTexture.hpp"

namespace ouzel::graphics::opengl
{
    namespace
    {
        std::string statusToString(GLenum status)
        {
            switch (status)
            {
                case GL_FRAMEBUFFER_COMPLETE:
                    return "GL_FRAMEBUFFER_COMPLETE";
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                    return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
#ifdef OUZEL_OPENGLES
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
#else
                case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
#endif
                    return "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS";
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                    return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
                case GL_FRAMEBUFFER_UNSUPPORTED:
                    return "GL_FRAMEBUFFER_UNSUPPORTED";
                default:
                    return std::to_string(status);
            }
        }
    }

    RenderTarget::RenderTarget(RenderDevice& initRenderDevice,
                               const std::set<Texture*>& initColorTextures,
                               Texture* initDepthTexture):
        RenderResource(initRenderDevice),
        colorTextures(initColorTextures),
        depthTexture(initDepthTexture)
    {
        createFrameBuffer();
    }

    RenderTarget::~RenderTarget()
    {
        if (frameBufferId)
            renderDevice.deleteFrameBuffer(frameBufferId);
    }

    void RenderTarget::reload()
    {
        for (Texture* colorTexture : colorTextures)
            colorTexture->reload();
        if (depthTexture) depthTexture->reload();

        frameBufferId = 0;
        createFrameBuffer();
    }

    void RenderTarget::createFrameBuffer()
    {
        renderDevice.glGenFramebuffersProc(1, &frameBufferId);

        GLenum error;

        if ((error = renderDevice.glGetErrorProc()) != GL_NO_ERROR)
            throw std::system_error(makeErrorCode(error), "Failed to upload texture data");

        for (Texture* colorTexture : colorTextures)
        {
            if (colorTexture)
            {
                auto index = static_cast<GLenum>(colorTextures.size() - 1);
                renderDevice.bindFrameBuffer(frameBufferId);

                if (colorTexture->getTextureId())
                    renderDevice.glFramebufferTexture2DProc(GL_FRAMEBUFFER,
                                                            GL_COLOR_ATTACHMENT0 + index,
                                                            GL_TEXTURE_2D,
                                                            colorTexture->getTextureId(), 0);
                else
                    renderDevice.glFramebufferRenderbufferProc(GL_FRAMEBUFFER,
                                                               GL_COLOR_ATTACHMENT0 + index,
                                                               GL_RENDERBUFFER,
                                                               colorTexture->getBufferId());

                if ((error = renderDevice.glGetErrorProc()) != GL_NO_ERROR)
                    throw std::system_error(makeErrorCode(error), "Failed to set frame buffer's color render buffer");

                GLenum status;
                if ((status = renderDevice.glCheckFramebufferStatusProc(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
                    throw std::runtime_error("Failed to create frame buffer, status: " + statusToString(status));

                if ((error = renderDevice.glGetErrorProc()) != GL_NO_ERROR)
                    throw std::system_error(makeErrorCode(error), "Failed to check frame buffer status");
            }
        }

        if (depthTexture)
        {
            renderDevice.bindFrameBuffer(frameBufferId);

            const GLenum attachment = (depthTexture->getPixelFormat() == GL_DEPTH_STENCIL) ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;

            renderDevice.glFramebufferTexture2DProc(GL_FRAMEBUFFER,
                                                    attachment,
                                                    GL_TEXTURE_2D,
                                                    depthTexture->getTextureId(), 0);
            //glFramebufferRenderbufferProc(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, texture->getBufferId());

            if ((error = renderDevice.glGetErrorProc()) != GL_NO_ERROR)
                throw std::system_error(makeErrorCode(error), "Failed to set frame buffer's depth render buffer");

            GLenum status;
            if ((status = renderDevice.glCheckFramebufferStatusProc(GL_FRAMEBUFFER)) != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error("Failed to create frame buffer, status: " + statusToString(status));

            if ((error = renderDevice.glGetErrorProc()) != GL_NO_ERROR)
                throw std::system_error(makeErrorCode(error), "Failed to check frame buffer status");
        }
    }
}

#endif
