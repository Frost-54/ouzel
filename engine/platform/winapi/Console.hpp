// Ouzel by Elviss Strazdins

#ifndef OUZEL_PLATFORM_WINAPI_CONSOLE_HPP
#define OUZEL_PLATFORM_WINAPI_CONSOLE_HPP

#include <system_error>
#include <ConsoleApi.h>

namespace ouzel::platform::winapi
{
    class Console final
    {
    public:
        Console():
            allocated{AllocConsole() != FALSE}
        {
            if (!allocated)
                throw std::system_error{GetLastError(), std::system_category(), "Failed to allocate console"};
        }

        ~Console()
        {
            if (allocated) FreeConsole();
        }

        Console(Console&& other) noexcept:
            allocated{other.allocated}
        {
            other.allocated = false;
        }

        Console& operator=(Console&& other) noexcept
        {
            if (&other == this) return *this;
            if (allocated) FreeConsole();
            allocated = other.allocated;
            other.allocated = false;
            return *this;
        }

    private:
        bool allocated = false;
    };
}

#endif // OUZEL_PLATFORM_WINAPI_CONSOLE_HPP
