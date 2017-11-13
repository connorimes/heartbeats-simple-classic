cmake_minimum_required(VERSION 2.8.9)

project(heartbeats-simple-classic)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 1)
set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if(CMAKE_HOST_UNIX)
  include(GNUInstallDirs)
else()
  set(CMAKE_INSTALL_LIBDIR lib)
  set(CMAKE_INSTALL_INCLUDEDIR include)
endif()

# force fPIC unless overridden (not automatically forwarded through OBJECT libraries when BUILD_SHARED_LIBS is set)
if (NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
endif()


# Dependencies

# Use OpenMP for timing
find_package(OpenMP REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HBS REQUIRED heartbeats-simple)
pkg_check_modules(ENERGYMON REQUIRED energymon-default)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} ${HBS_CFLAGS_OTHER} ${ENERGYMON_CFLAGS_OTHER} -Wall -std=c99")

include_directories(${PROJECT_SOURCE_DIR}/inc ${HBS_INCLUDE_DIRS} ${ENERGYMON_INCLUDE_DIRS})


# Libraries

add_library(hbsc OBJECT src/hbs-classic.c)

add_library(hbsc-acc OBJECT src/hbs-classic.c)
set_target_properties(hbsc-acc PROPERTIES COMPILE_DEFINITIONS "HEARTBEAT_MODE_ACC")

add_library(hbsc-pow OBJECT src/hbs-classic.c)
set_target_properties(hbsc-pow PROPERTIES COMPILE_DEFINITIONS "HEARTBEAT_MODE_POW")

add_library(hbsc-acc-pow OBJECT src/hbs-classic.c)
set_target_properties(hbsc-acc-pow PROPERTIES COMPILE_DEFINITIONS "HEARTBEAT_MODE_ACC_POW")

add_library(heartbeats-simple-classic $<TARGET_OBJECTS:hbsc>
                                      $<TARGET_OBJECTS:hbsc-acc>
                                      $<TARGET_OBJECTS:hbsc-pow>
                                      $<TARGET_OBJECTS:hbsc-acc-pow>)
target_link_libraries(heartbeats-simple-classic -L${HBS_LIBDIR} ${HBS_LIBRARIES}
                                                -L${ENERGYMON_LIBDIR} ${ENERGYMON_LIBRARIES})
if(BUILD_SHARED_LIBS)
  set_target_properties(heartbeats-simple-classic PROPERTIES VERSION ${PROJECT_VERSION}
                                                             SOVERSION ${VERSION_MAJOR})
endif()


# Tests

add_executable(hbs-classic-test test/hbs-classic-test.c)
target_link_libraries(hbs-classic-test heartbeats-simple-classic)

enable_testing()
macro(add_unit_test target)
  add_test(${target} ${EXECUTABLE_OUTPUT_PATH}/${target})
endmacro(add_unit_test)

add_unit_test(hbs-classic-test)


# pkg-config

set(PKG_CONFIG_EXEC_PREFIX "\${prefix}")
set(PKG_CONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")
set(PKG_CONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}")
set(PKG_CONFIG_CFLAGS "-I\${includedir} ${OpenMP_C_FLAGS}")

set(PKG_CONFIG_NAME "heartbeats-simple-classic")
set(PKG_CONFIG_DESCRIPTION "A classic Heartbeats style interface to heartbeats-simple")
set(PKG_CONFIG_REQUIRES "heartbeats-simple >= 0.3.0, energymon-default >= 0.2.0")
set(PKG_CONFIG_LIBS "-L\${libdir} -lheartbeats-simple-classic")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig.in
  ${CMAKE_CURRENT_BINARY_DIR}/pkgconfig/heartbeats-simple-classic.pc
)


# Install

install(TARGETS heartbeats-simple-classic DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY inc/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(DIRECTORY ${CMAKE_BINARY_DIR}/pkgconfig/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)


# Uninstall

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
  @ONLY
)

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
