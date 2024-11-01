#
# Copyright 2024, DornerWorks 
#
# SPDX-License-Identifier: BSD-2-Clause
#

project(usb C)

CamkESAddCPPInclude("${CMAKE_CURRENT_LIST_DIR}/include/")

set(CAmkESCPP ON CACHE BOOL "" FORCE)
if("${KernelArch}" STREQUAL "x86")
    set(cpp_define -DCONFIG_ARCH_X86 -DCONFIG_IOMMU -DCONFIG_PLAT_PC99)
elseif("${KernelArch}" STREQUAL "arm")
    set(cpp_define -DCONFIG_ARCH_ARM)
endif()


DeclareCAmkESComponent(
    Driver
    SOURCES
    components/Driver/src/drv.c
    components/Driver/src/cdc.c
    components/Driver/src/hid.c
    LIBS
    usbdrivers sel4simple pci sel4allocman

)
DeclareCAmkESComponent(
    Pl2303
    SOURCES
    components/Pl2303/src/main.c
)

DeclareCAmkESComponent(
    Touchscreen
    SOURCES
    components/Touchscreen/src/main.c
)

DeclareCAmkESComponent(
    Keyboard
    SOURCES
    components/Keyboard/src/main.c
)
# DeclareCAmkESComponent(
#     Probe
#     SOURCES
#     components/Probe/src/main.c
# )

# DeclareCAmkESComponent(
#     Flipper
#     SOURCES
#     components/Flipper/src/main.c
#     components/Flipper/src/lib_crc.c
#     components/Flipper/src/flipper.c
#     INCLUDES
#     include/
# )
# DeclareCAmkESComponent(
#     Mobility
#     SOURCES
#     components/Mobility/src/main.c
#     components/Mobility/src/mobility.c
#     components/Mobility/src/lib_crc.c
#     INCLUDES
#     include/
# )

includeGlobalComponents()

DeclareCAmkESRootserver(
    usb.camkes
    CPP_FLAGS
    ${cpp_define}
)
