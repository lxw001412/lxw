# 设置目标系统
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm64)

# 设置工具链目录
set(TOOL_CHAIN_DIR $ENV{CROSSTOOL_PATH})
set(TOOL_CHAIN_INCLUDE 
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/libc/usr/include
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/include
    )
set(TOOL_CHAIN_LIB 
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/libc/usr/lib
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/lib
    )


# 设置编译器位置
set(CMAKE_C_COMPILER $ENV{CROSSTOOL_CMD}gcc)
set(CMAKE_CXX_COMPILER $ENV{CROSSTOOL_CMD}g++)

# 设置Cmake查找主路径
set(CMAKE_FIND_ROOT_PATH ${TOOL_CHAIN_DIR}/aarch64-linux-gnu)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# 只在指定目录下查找库文件
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
# 只在指定目录下查找头文件
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# 只在指定目录下查找依赖包
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

include_directories(
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/include
    ${TOOL_CHAIN_DIR}/aarch64-linux-gnu/libc/usr/include
    )

set(CMAKE_INCLUDE_PATH 
    ${TOOL_CHAIN_INCLUDE}
    )

set(CMAKE_LIBRARY_PATH 
    ${TOOL_CHAIN_LIB}
    )
