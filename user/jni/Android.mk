LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := Kernel_Hack
LOCAL_SRC_FILES := KernelRW.cpp main.cpp
LOCAL_CPPFLAGS  := -std=c++17 -O2
LOCAL_LDLIBS    :=

include $(BUILD_EXECUTABLE)