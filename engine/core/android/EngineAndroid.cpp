// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include <cstdlib>
#include <unistd.h>
#include <android/window.h>
#include "EngineAndroid.hpp"
#include "NativeWindowAndroid.hpp"
#include "../../events/EventDispatcher.hpp"
#include "../../graphics/opengl/android/OGLRenderDeviceAndroid.hpp"

namespace ouzel
{
    namespace
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

        const ErrorCategory errorCategory {};

        int looperCallback(int fd, int events, void* data)
        {
            if (events & ALOOPER_EVENT_INPUT)
            {
                char command;
                if (read(fd, &command, sizeof(command)) == -1)
                    throw std::system_error(errno, std::system_category(), "Failed to read from pipe");

                auto engineAndroid = static_cast<EngineAndroid*>(data);

                if (command == 1)
                    engineAndroid->executeAll();
            }

            return 1;
        }
    }

    const std::error_category& getErrorCategory() noexcept
    {
        return errorCategory;
    }

    EngineAndroid::EngineAndroid(JavaVM* initJavaVm):
        javaVm(initJavaVm)
    {
        void* jniEnvPointer;

        jint result;
        if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to get JNI environment");

        auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

        uriClass = static_cast<jclass>(jniEnv->NewGlobalRef(jniEnv->FindClass("android/net/Uri")));
        parseMethod = jniEnv->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
        intentClass = static_cast<jclass>(jniEnv->NewGlobalRef(jniEnv->FindClass("android/content/Intent")));
        intentConstructor = jniEnv->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
    }

    EngineAndroid::~EngineAndroid()
    {
        if (updateThread.isJoinable()) updateThread.join();

        void* jniEnvPointer;

        if (javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6) == JNI_OK)
        {
            auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

            if (mainActivity) jniEnv->DeleteGlobalRef(mainActivity);
            if (androidWindow) jniEnv->DeleteGlobalRef(androidWindow);
            if (surface) jniEnv->DeleteGlobalRef(surface);
            if (intentClass) jniEnv->DeleteGlobalRef(intentClass);
            if (uriClass) jniEnv->DeleteGlobalRef(uriClass);
        }

        if (looper) ALooper_release(looper);
        if (looperPipe[0] != -1) close(looperPipe[0]);
        if (looperPipe[1] != -1) close(looperPipe[1]);
    }

    void EngineAndroid::onCreate(jobject initMainActivity)
    {
        void* jniEnvPointer;

        jint result;
        if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to get JNI environment");

        auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

        mainActivity = jniEnv->NewGlobalRef(initMainActivity);

        jclass mainActivityClass = jniEnv->GetObjectClass(mainActivity);
        startActivityMethod = jniEnv->GetMethodID(mainActivityClass, "startActivity", "(Landroid/content/Intent;)V");

        // get resources
        jclass resourcesClass = jniEnv->FindClass("android/content/res/Resources");
        jmethodID getResourcesMethod = jniEnv->GetMethodID(mainActivityClass, "getResources", "()Landroid/content/res/Resources;");
        jobject resourcesObject = jniEnv->CallObjectMethod(mainActivity, getResourcesMethod);
        jmethodID getConfigurationMethod = jniEnv->GetMethodID(resourcesClass, "getConfiguration", "()Landroid/content/res/Configuration;");

        // get configuration
        jclass configurationClass = jniEnv->FindClass("android/content/res/Configuration");
        jobject configurationObject = jniEnv->CallObjectMethod(resourcesObject, getConfigurationMethod);

        orientationField = jniEnv->GetFieldID(configurationClass, "orientation", "I");
        orientation = jniEnv->GetIntField(configurationObject, orientationField);

        // get asset manager
        jmethodID getAssetsMethod = jniEnv->GetMethodID(mainActivityClass, "getAssets", "()Landroid/content/res/AssetManager;");
        jobject assetManagerObject = jniEnv->CallObjectMethod(mainActivity, getAssetsMethod);
        assetManager = AAssetManager_fromJava(jniEnv, assetManagerObject);

        // get window
        jmethodID getWindowMethod = jniEnv->GetMethodID(mainActivityClass, "getWindow", "()Landroid/view/Window;");
        androidWindow = jniEnv->CallObjectMethod(mainActivity, getWindowMethod);
        androidWindow = jniEnv->NewGlobalRef(androidWindow);

        jclass windowClass = jniEnv->FindClass("android/view/Window");
        addFlagsMethod = jniEnv->GetMethodID(windowClass, "addFlags", "(I)V");
        clearFlagsMethod = jniEnv->GetMethodID(windowClass, "clearFlags", "(I)V");

        // File class
        jclass fileClass = jniEnv->FindClass("java/io/File");
        jmethodID getAbsolutePathMethod = jniEnv->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

        // dataDir
        jmethodID getFilesDirMethod = jniEnv->GetMethodID(mainActivityClass, "getFilesDir", "()Ljava/io/File;");
        jobject filesDirFile = jniEnv->CallObjectMethod(mainActivity, getFilesDirMethod);

        auto filesDirString = static_cast<jstring>(jniEnv->CallObjectMethod(filesDirFile, getAbsolutePathMethod));
        auto filesDirCString = jniEnv->GetStringUTFChars(filesDirString, 0);
        filesDirectory = filesDirCString;
        jniEnv->ReleaseStringUTFChars(filesDirString, filesDirCString);

        // cacheDir
        jmethodID getCacheDirMethod = jniEnv->GetMethodID(mainActivityClass, "getCacheDir", "()Ljava/io/File;");
        jobject cacheDirFile = jniEnv->CallObjectMethod(mainActivity, getCacheDirMethod);

        auto cacheDirString = static_cast<jstring>(jniEnv->CallObjectMethod(cacheDirFile, getAbsolutePathMethod));
        auto cacheDirCString = jniEnv->GetStringUTFChars(cacheDirString, 0);
        cacheDirectory = cacheDirCString;
        jniEnv->ReleaseStringUTFChars(cacheDirString, cacheDirCString);

        // looper
        looper = ALooper_forThread(); // this is called on main thread, so it is safe to get the looper here
        if (!looper)
            throw std::runtime_error("Main thread has no looper");

        ALooper_acquire(looper);
        if (pipe(looperPipe) == -1)
            throw std::system_error(errno, std::system_category(), "Failed to create pipe");

        if (ALooper_addFd(looper, looperPipe[0], ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT, looperCallback, this) != 1)
            throw std::runtime_error("Failed to add looper file descriptor");
    }

    void EngineAndroid::onSurfaceCreated(jobject newSurface)
    {
        void* jniEnvPointer;

        jint result;
        if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to get JNI environment");

        auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

        if (surface) jniEnv->DeleteGlobalRef(surface);
        surface = jniEnv->NewGlobalRef(newSurface);

        if (active)
        {
            auto windowAndroid = static_cast<NativeWindowAndroid*>(window->getNativeWindow());
            windowAndroid->handleSurfaceChange(surface);

            if (renderer)
            {
                graphics::RenderDevice* renderDevice = renderer->getDevice();
                if (renderDevice->getDriver() == graphics::Driver::openGL)
                {
                    auto renderDeviceOGLAndroid = static_cast<graphics::opengl::RenderDeviceAndroid*>(renderDevice);
                    renderDeviceOGLAndroid->reload();
                }
            }
        }
    }

    void EngineAndroid::onConfigurationChanged(jobject newConfig)
    {
        void* jniEnvPointer;

        jint result;
        if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to get JNI environment");

        auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

        jint newOrientation = jniEnv->GetIntField(newConfig, orientationField);

        if (orientation != newOrientation)
        {
            orientation = newOrientation;

            auto event = std::make_unique<SystemEvent>();
            event->type = Event::Type::orientationChange;

            switch (orientation)
            {
                case ORIENTATION_PORTRAIT:
                    event->orientation = SystemEvent::Orientation::portrait;
                    break;
                case ORIENTATION_LANDSCAPE:
                    event->orientation = SystemEvent::Orientation::landscape;
                    break;
                default: // unsupported orientation, assume portrait
                    event->orientation = SystemEvent::Orientation::portrait;
                    break;
            }

            eventDispatcher.postEvent(std::move(event));
        }
    }

    void EngineAndroid::onSurfaceDestroyed()
    {
        void* jniEnvPointer;

        jint result;
        if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to get JNI environment");

        auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

        if (surface)
        {
            jniEnv->DeleteGlobalRef(surface);
            surface = nullptr;
        }

        if (active)
        {
            auto windowAndroid = static_cast<NativeWindowAndroid*>(window->getNativeWindow());
            windowAndroid->handleSurfaceDestroy();

            if (renderer)
            {
                graphics::RenderDevice* renderDevice = renderer->getDevice();
                if (renderDevice->getDriver() == graphics::Driver::openGL)
                {
                    auto renderDeviceOGLAndroid = static_cast<graphics::opengl::RenderDeviceAndroid*>(renderDevice);
                    renderDeviceOGLAndroid->destroy();
                }
            }
        }
    }

    void EngineAndroid::run()
    {
        init();
        start();
    }

    void EngineAndroid::runOnMainThread(const std::function<void()>& func)
    {
        std::unique_lock<std::mutex> lock(executeMutex);
        executeQueue.push(func);
        lock.unlock();

        const std::uint8_t command = 1;
        if (write(looperPipe[1], &command, sizeof(command)) == -1)
            throw std::system_error(errno, std::system_category(), "Failed to write to pipe");
    }

    void EngineAndroid::openUrl(const std::string& url)
    {
        executeOnMainThread([url, this]() {
            void* jniEnvPointer;

            jint result;
            if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
                throw std::system_error(result, errorCategory, "Failed to get JNI environment");

            auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

            jstring actionString = jniEnv->NewStringUTF("android.intent.action.VIEW");
            jstring urlString = jniEnv->NewStringUTF(url.c_str());
            jobject uri = jniEnv->CallStaticObjectMethod(uriClass, parseMethod, urlString);
            jobject intentObject = jniEnv->NewObject(intentClass, intentConstructor, actionString, uri);

            jniEnv->CallVoidMethod(mainActivity, startActivityMethod, intentObject);

            jniEnv->DeleteLocalRef(intentObject);
            jniEnv->DeleteLocalRef(uri);
            jniEnv->DeleteLocalRef(urlString);
            jniEnv->DeleteLocalRef(actionString);

            if (jniEnv->ExceptionCheck())
            {
                jniEnv->ExceptionClear();
                throw std::runtime_error("Failed to open URL");
            }
        });
    }

    void EngineAndroid::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        Engine::setScreenSaverEnabled(newScreenSaverEnabled);

        executeOnMainThread([newScreenSaverEnabled, this]() {
            void* jniEnvPointer;

            jint result;
            if ((result = javaVm->GetEnv(&jniEnvPointer, JNI_VERSION_1_6)) != JNI_OK)
                throw std::system_error(result, errorCategory, "Failed to get JNI environment");

            auto jniEnv = static_cast<JNIEnv*>(jniEnvPointer);

            if (newScreenSaverEnabled)
                jniEnv->CallVoidMethod(androidWindow, clearFlagsMethod, AWINDOW_FLAG_KEEP_SCREEN_ON);
            else
                jniEnv->CallVoidMethod(androidWindow, addFlagsMethod, AWINDOW_FLAG_KEEP_SCREEN_ON);
        });
    }

    void EngineAndroid::executeAll()
    {
        std::function<void()> func;

        for (;;)
        {
            std::unique_lock<std::mutex> lock(executeMutex);
            if (executeQueue.empty()) break;

            func = std::move(executeQueue.front());
            executeQueue.pop();
            lock.unlock();

            if (func) func();
        }
    }

    void EngineAndroid::engineMain()
    {
        JNIEnv* jniEnv;
        JavaVMAttachArgs attachArgs;
        attachArgs.version = JNI_VERSION_1_6;
        attachArgs.name = "Application"; // thread name
        attachArgs.group = nullptr; // thread group

        jint result;
        if ((result = javaVm->AttachCurrentThread(&jniEnv, &attachArgs)) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to attach current thread to Java VM");

        Engine::engineMain();

        if ((result = javaVm->DetachCurrentThread()) != JNI_OK)
            throw std::system_error(result, errorCategory, "Failed to detach current thread from Java VM");
    }
}
