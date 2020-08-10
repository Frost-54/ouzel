// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <cstdlib>
#include "SystemTVOS.hpp"
#include "EngineTVOS.hpp"
#include "../../utils/Log.hpp"

int main(int argc, char* argv[])
{
    try
    {
        ouzel::core::tvos::System system(argc, argv);

        ouzel::core::tvos::Engine engine(argc, argv);
        engine.run();
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        ouzel::logger.log(ouzel::Log::Level::error) << e.what();
        return EXIT_FAILURE;
    }
}

namespace ouzel::tvos
{
}
