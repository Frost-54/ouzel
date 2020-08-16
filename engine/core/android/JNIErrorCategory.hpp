// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_CORE_JNIERRORCATEGORY_HPP
#define OUZEL_CORE_JNIERRORCATEGORY_HPP

#include <system_error>
#include <jni.h>

namespace ouzel::core::android
{
    class ErrorCategory final: public std::error_category
    {
    public:
        const char* name() const noexcept final
        {
            return "JNI";
        }

        std::string message(int condition) const final
        {
            switch (condition)
            {
                case JNI_ERR: return "JNI_ERR";
                case JNI_EDETACHED: return "JNI_EDETACHED";
                case JNI_EVERSION: return "JNI_EVERSION";
                default: return "Unknown error (" + std::to_string(condition) + ")";
            }
        }
    };
}

#endif // OUZEL_CORE_JNIERRORCATEGORY_HPP
