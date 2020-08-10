// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <cstdlib>
#include "SystemTVOS.hpp"
#include "EngineTVOS.hpp"
#include "../cocoa/AutoreleasePool.hpp"
#include "../../utils/Log.hpp"

int main(int argc, char* argv[])
{
    try
    {
        ouzel::core::cocoa::AutoreleasePool autoreleasePool;
        
        ouzel::core::tvos::System system(argc, argv);

        ouzel::core::tvos::Engine engine(argc, argv);
        engine.run(argc, argv);
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        ouzel::logger.log(ouzel::Log::Level::error) << e.what();
        return EXIT_FAILURE;
    }
}

namespace ouzel::core::tvos
{
    namespace
    {
        std::vector<std::string> parseArgs(int argc, char* argv[])
        {
            std::vector<std::string> result;
            for (int i = 0; i < argc; ++i)
                result.push_back(argv[i]);
            return result;
        }
    }

    System::System(int argc, char* argv[]):
        core::System(parseArgs(argc, argv))
    {
    }
}
