LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := streaming-fragment
LOCAL_SRC_FILES := streaming-fragment.c
LOCAL_SHARED_LIBRARIES := gstreamer_android
LOCAL_LDLIBS := -llog -landroid
include $(BUILD_SHARED_LIBRARY)

ifndef GSTREAMER_SDK_ROOT
ifndef GSTREAMER_ROOT
$(error GSTREAMER_ROOT is not defined!)
endif
GSTREAMER_SDK_ROOT        := $(GSTREAMER_ROOT)
endif
GSTREAMER_NDK_BUILD_PATH  := $(GSTREAMER_SDK_ROOT)/share/gst-android/ndk-build/
include $(GSTREAMER_NDK_BUILD_PATH)/plugins.mk

GSTREAMER_PLUGINS         := $(GSTREAMER_PLUGINS_CORE) $(GSTREAMER_PLUGINS_PLAYBACK) $(GSTREAMER_PLUGINS_CODECS) $(GSTREAMER_PLUGINS_NET) $(GSTREAMER_PLUGINS_SYS) $(GSTREAMER_PLUGINS_CODECS_RESTRICTED) $(GSTREAMER_PLUGINS_BAD)
GSTREAMER_EXTRA_DEPS      :=  gstreamer-video-1.0
include $(GSTREAMER_NDK_BUILD_PATH)/gstreamer-1.0.mk
