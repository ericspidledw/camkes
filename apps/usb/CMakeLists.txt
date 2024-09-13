#
# Copyright 2024, DornerWorks 
#
# SPDX-License-Identifier: BSD-2-Clause
#

project(usb C)

CamkESAddCPPInclude("${CMAKE_CURRENT_LIST_DIR}/include/")

set(CAmkESCPP ON CACHE BOOL "" FORCE)
if("${KernelArch}" STREQUAL "x86")
    set(cpp_define -DKernelArchX86)
elseif("${KernelArch}" STREQUAL "arm")
    set(cpp_define -DKernelArchArm)
endif()


DeclareCAmkESComponent(
    Driver
    SOURCES
    components/Driver/src/drv.c
    components/Driver/src/cdc.c
    LIBS
    usbdrivers sel4simple pci sel4allocman

)
DeclareCAmkESComponent(
    Flipper
    SOURCES
    components/Flipper/src/main.c
    # components/Flipper/src/lib_crc.c
    # components/Flipper/src/flipper.c

    INCLUDES
    include/lib_crc.h
)
DeclareCAmkESComponent(
    Mobility
    SOURCES
    components/Mobility/src/main.c
    INCLUDES
    include/lib_crc.h
)

includeGlobalComponents()

DeclareCAmkESRootserver(
    usb.camkes
    CPP_FLAGS
    ${cpp_define}
)
