LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main
LOCAL_CFLAGS := -Wall -Wextra -Wshadow -Wdouble-promotion
LOCAL_CPPFLAGS += -std=c++14 -fexceptions
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../engine

LOCAL_SRC_FILES := ../../../../AnimationsSample.cpp \
    ../../../../GUISample.cpp \
    ../../../../GameSample.cpp \
    ../../../../InputSample.cpp \
    ../../../../main.cpp \
    ../../../../MainMenu.cpp \
    ../../../../PerspectiveSample.cpp \
    ../../../../SoundSample.cpp \
    ../../../../SpritesSample.cpp \
    ../../../../RTSample.cpp

LOCAL_WHOLE_STATIC_LIBRARIES := ouzel
LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -lEGL -llog -landroid -lOpenSLES -latomic

include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/../../../../../engine/jni/Android.mk