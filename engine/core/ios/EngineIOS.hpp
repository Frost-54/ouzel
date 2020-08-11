// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_CORE_ENGINEIOS_HPP
#define OUZEL_CORE_ENGINEIOS_HPP

#if defined(__OBJC__)
@class ExecuteHandler;
typedef ExecuteHandler* ExecuteHandlerPtr;
#else
#  include <objc/NSObjCRuntime.h>
typedef id ExecuteHandlerPtr;
#endif

#include "../Engine.hpp"

namespace ouzel::core::ios
{
    class Engine final: public core::Engine
    {
    public:
        Engine(int argc, char* argv[]);
        ~Engine() override;

        void run(int argc, char* argv[]);

        void openUrl(const std::string& url) final;

        void setScreenSaverEnabled(bool newScreenSaverEnabled) final;

        void executeAll();

    private:
        void runOnMainThread(const std::function<void()>& func) final;

        std::queue<std::function<void()>> executeQueue;
        std::mutex executeMutex;

        ExecuteHandlerPtr executeHanlder = nil;
    };
}

#endif // OUZEL_CORE_ENGINEIOS_HPP
