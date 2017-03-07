// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include "Fade.h"
#include "scene/Node.h"

namespace ouzel
{
    namespace scene
    {
        Fade::Fade(float aLength, float aOpacity, bool aRelative):
            Animator(aLength), opacity(aOpacity), relative(aRelative)
        {
        }

        void Fade::play()
        {
            Animator::play();

            if (std::shared_ptr<Node> node = targetNode.lock())
            {
                startOpacity = node->getOpacity();
                targetOpacity = relative ? startOpacity + opacity : opacity;

                diff = targetOpacity - startOpacity;
            }
        }

        void Fade::updateProgress()
        {
            Animator::updateProgress();

            if (std::shared_ptr<Node> node = targetNode.lock())
            {
                node->setOpacity(startOpacity + (diff * progress));
            }
        }
    } // namespace scene
} // namespace ouzel
