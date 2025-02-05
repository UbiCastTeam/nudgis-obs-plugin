cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

# Custom prameters ------------------------------------------------------------
# Disable linux portable mode otherwise plugin get installed in folder not looked upon by OBS (obs-plugins/{32,64}bits)
set(LINUX_PORTABLE OFF)
# set a custom package version for debian .deb
set(CPACK_DEBIAN_PACKAGE_VERSION "${_version}+OBS-31.0.0")
# -----------------------------------------------------------------------------

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

# Import dependencies
find_package(libobs REQUIRED)
find_package(CURL REQUIRED)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME}
    PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON
  )
endif()

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)
target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/nudgis-plugin.cpp
          src/nudgis-service.cpp
          src/nudgis-config.cpp
          src/nudgis-upload-ui.cpp
          src/http-client.cpp
          src/obs/obs-utils.cpp
          src/plugin-macros.generated.h)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/obs)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs CURL::libcurl Qt6::Core Qt6::Widgets)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})

# --- Tests ---

add_executable(test)

target_sources(
  test
  PRIVATE src/nudgis-plugin.cpp
          src/nudgis-service.cpp
          src/nudgis-config.cpp
          src/nudgis-upload-ui.cpp
          src/http-client.cpp
          src/obs/obs-utils.cpp
          src/test.cpp)

target_link_libraries(test PRIVATE OBS::libobs OBS::obs-frontend-api CURL::libcurl Qt6::Core Qt6::Widgets)

set_target_properties(
  test
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_include_directories(test PRIVATE ${CMAKE_SOURCE_DIR}/src/obs ${CMAKE_SOURCE_DIR}/src)
