# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#设置生成的可执行文件的路径
# 以Windows系统下生成armeabi-v7a平台的可执行文件为例
# app\.cxx\cmake\debug\armeabi-v7a\src\main\assets\mmc
set(EXECUTABLE_OUTPUT_PATH  "${CMAKE_CURRENT_SOURCE_DIR}/../assets/${ANDROID_ABI}")
# set(EXECUTABLE_OUTPUT_PATH  "app/build/intermediates/ndkBuild/debug/obj/local/")${ANDROID_ABI}
#

# 头文件
include_directories(
        /src/main/cpp/includes/    # NDK
        /src/main/cpp/3rdparty/hmac_sha/   # 源码hmac_sha下的头文件
        /src/main/cpp/  #源码cpp下的头文件
)


# 将源码编译为指定名称的可执行文件
#
add_executable(
        mmc # 生成名为mmc的可执行文件

        lsmmc.c
        mmc.c
        mmc_cmds.c
        my_cmds.c
        3rdparty/hmac_sha/hmac_sha2.c
        3rdparty/hmac_sha/sha2.c)

# 添加目标编译目录
target_include_directories (mmc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
