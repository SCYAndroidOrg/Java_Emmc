#固定写法
LOCAL_PATH:=$(call my-dir)
#固定写法
include $(CLEAR_VARS)
#生成so名称
LOCAL_MODULE := JniHelper
LOCAL_SRC_FILES := JniUtil2.cpp
#固定写法
include $(BUILD_SHARED_LIBRARY)
