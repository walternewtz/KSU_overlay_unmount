LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ksuhide
LOCAL_SRC_FILES := main.cpp magiskhide.cpp utils.cpp magiskhide_util.cpp mntinfo.cpp nicename.cpp logging.cpp magic-mount/main.cpp magic-mount/utils.cpp
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

include jni/libcxx/Android.mk
