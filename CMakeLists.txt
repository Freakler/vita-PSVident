cmake_minimum_required(VERSION 2.8)

# VitaSDK defines
if( NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  if( DEFINED ENV{VITASDK} )
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
  message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()


# Project start
project(PSVident)
include("${VITASDK}/share/vita.cmake" REQUIRED)
set(VITA_APP_NAME "PSVident")
set(VITA_TITLEID  "PSVIDENT0")
set(VITA_VERSION  "01.00")


# Flags and includes
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable -Wno-format-truncation -fno-lto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c+ + 11 -fno-rtti -fno-exceptions")
set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1")
set(VITA_MAKE_FSELF_FLAGS "${VITA_MAKE_FSELF_FLAGS} -a 0x2808000000000000")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/kernel_stubs
  ${CMAKE_BINARY_DIR}/user_stubs
  ${CMAKE_BINARY_DIR}/nids/add_nids
)

add_executable(${PROJECT_NAME}
  src/main.c
  src/utils/aes.c
  src/utils/utils.c
  src/utils/registry.c
  src/print/scr_printf.c
  src/print/font.c
  src/psvident.c
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_kernel.skprx)

target_link_libraries(${PROJECT_NAME}
  SceSysmodule_stub
  SceVshBridge_stub
  SceCtrl_stub
  SceDisplay_stub
  SceNet_stub
  SceNetCtl_stub
  SceRtc_stub
  SceApputil_stub
  SceSysmem_stub
  ScePower_stub
  SceRegistryMgr_stub
  SceSysrootForKernel_stub
  SceSysrootForDriver_stub
  SceSblSsMgr_stub
  SceNotificationUtil_stub
  SceMotionDev_stub
  taihen_stub
  psvident_user_stub_weak
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)


### ### ### ### ### ### ### ### ### 
add_executable(${PROJECT_NAME}_kernel
  src/plugin/kernel.c
)
set_target_properties(${PROJECT_NAME}_kernel
  PROPERTIES LINK_FLAGS
  -nostdlib
)
target_link_libraries(${PROJECT_NAME}_kernel
  SceAppMgrForDriver_stub
  SceCtrlForDriver_stub
  SceCpuForDriver_stub
  SceIofilemgrForDriver_stub
  SceKernelUtilsForDriver_stub
  SceModulemgrForDriver_stub
  SceSblSsMgrForDriver_stub
  SceSysclibForDriver_stub
  SceSysmemForDriver_stub
  SceSysrootForKernel_stub
  SceThreadmgrForDriver_stub
  SceSysrootForDriver_stub 
  SceSysconForDriver_stub
  ScePervasiveForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
)
vita_create_self(${PROJECT_NAME}_kernel.skprx ${PROJECT_NAME}_kernel
  CONFIG ${CMAKE_SOURCE_DIR}/src/plugin/kernel_exports.yml
  UNSAFE
)
vita_create_stubs(kernel_stubs ${PROJECT_NAME}_kernel ${CMAKE_SOURCE_DIR}/src/plugin/kernel_exports.yml KERNEL)

### ### ### ### ### ### ### ### ### ### 

add_executable(${PROJECT_NAME}_user
  src/plugin/user.c
)
set_target_properties(${PROJECT_NAME}_user
  PROPERTIES LINK_FLAGS
  -nostartfiles
)
target_link_libraries(${PROJECT_NAME}_user
  SceAppMgr_stub
  SceShellSvc_stub
  ScePower_stub
  taihen_stub
  psvident_kernel_stub
)
vita_create_self(${PROJECT_NAME}_user.suprx ${PROJECT_NAME}_user
  CONFIG ${CMAKE_SOURCE_DIR}/src/plugin/user_exports.yml
  UNSAFE
)
vita_create_stubs(user_stubs ${PROJECT_NAME}_user ${CMAKE_SOURCE_DIR}/src/plugin/user_exports.yml)

### ### ### ### ### ### ### ### ### ### 

add_dependencies(${PROJECT_NAME}_user kernel_stubs)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_kernel.skprx)
add_dependencies(${PROJECT_NAME} user_stubs)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_user.suprx)

vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
	FILE ${PROJECT_NAME}_kernel.skprx ${PROJECT_NAME}_kernel.skprx
	FILE ${PROJECT_NAME}_user.suprx ${PROJECT_NAME}_user.suprx
	FILE livearea/icon0.png sce_sys/icon0.png
    FILE livearea/bg0.png sce_sys/livearea/contents/bg0.png
    FILE livearea/startup.png sce_sys/livearea/contents/startup.png
    FILE livearea/template.xml sce_sys/livearea/contents/template.xml
)

set(VITA_IP  "192.168.178.28")

add_custom_target(vpksend
	COMMAND curl -T ${PROJECT_NAME}.vpk ftp://${VITA_IP}:1337/ux0:/
	DEPENDS ${PROJECT_NAME}.vpk
)

add_custom_target(send
	COMMAND curl -T ${PROJECT_NAME}.self ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/eboot.bin #$(PSVITAIP)
	DEPENDS ${PROJECT_NAME}.self
)

add_custom_target(skprxsend
	COMMAND curl -T ${PROJECT_NAME}_kernel.skprx ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/${PROJECT_NAME}_kernel.skprx
	DEPENDS ${PROJECT_NAME}_kernel.skprx
)

add_custom_target(suprxsend
	COMMAND curl -T ${PROJECT_NAME}_user.suprx ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/${PROJECT_NAME}_user.suprx
	DEPENDS ${PROJECT_NAME}_user.suprx
)
add_custom_target(sendall
	COMMAND curl -T ${PROJECT_NAME}.self ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/eboot.bin #$(PSVITAIP)
	DEPENDS ${PROJECT_NAME}.self
	COMMAND curl -T ${PROJECT_NAME}_user.suprx ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/${PROJECT_NAME}_user.suprx
	DEPENDS ${PROJECT_NAME}_user.suprx
	COMMAND curl -T ${PROJECT_NAME}_kernel.skprx ftp://${VITA_IP}:1337/ux0:/app/${VITA_TITLEID}/${PROJECT_NAME}_kernel.skprx
	DEPENDS ${PROJECT_NAME}_kernel.skprx
)
add_custom_target(copy
  COMMAND cp ${PROJECT_NAME}.self G:/app/${VITA_TITLEID}/eboot.bin
  DEPENDS ${PROJECT_NAME}.self
)