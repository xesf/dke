# PlayStation Vita build, on VitaSDK's toolchain file.
#
#   make vita        (from the repository root; needs VitaSDK in ~/vitasdk or $VITASDK)
#
# Makes build/vita/dke.vpk. The game data goes to ux0:data/dke.

cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain")
    else()
        message(FATAL_ERROR "VITASDK is not set")
    endif()
endif()

project(dke C)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
file(GLOB SRC
    ${ROOT}/src/*.c
    ${ROOT}/src/libsys/*.c
    ${ROOT}/src/libsys/audio/*.c
    ${ROOT}/src/libsys/graphics/*.c
    ${ROOT}/src/levels/*.c
    ${ROOT}/src/menu/*.c)

add_executable(dke ${SRC} ${ROOT}/src/libsys/platforms/vita.c)
target_include_directories(dke PRIVATE ${ROOT}/src ${ROOT}/src/libsys)
set_property(TARGET dke PROPERTY C_STANDARD 99)
target_compile_definitions(dke PRIVATE PLATFORM_VITA _DEFAULT_SOURCE)
# -fsigned-char: the engine was written for x86, where char is signed.
target_compile_options(dke PRIVATE -O3 -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fno-math-errno
                       -Wall -fwrapv -fsigned-char
                       -Wno-missing-braces -Wno-unused-function -Wno-unused-variable)
target_link_libraries(dke SceDisplay_stub SceCtrl_stub SceAudio_stub
                      ScePower_stub SceSysmem_stub SceKernelThreadMgr_stub pthread m)

vita_create_self(dke.self dke)
vita_create_vpk(dke.vpk DKE000001 dke.self VERSION 01.00 NAME "Dark Earth")
