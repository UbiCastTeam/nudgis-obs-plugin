cmake_minimum_required(VERSION 3.16...3.21)

project(nudgis-obs-plugin VERSION 1.0.0)
add_library(${CMAKE_PROJECT_NAME} MODULE)

set(PLUGIN_AUTHOR "UbiCast SA")

set(MACOS_BUNDLEID "eu.ubicast.${CMAKE_PROJECT_NAME}")

set(LINUX_MAINTAINER_EMAIL "noreply@ubicast.eu")

# Add your custom source files here - header files are optional and only
# required for visibility e.g. in Xcode or Visual Studio
target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/nudgis-plugin.cpp src/nudgis-service.cpp src/nudgis-config.cpp
          src/nudgis-upload-ui.cpp src/http-client.cpp src/obs/obs-utils.cpp)

# Import libobs as main plugin dependency
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(libobs REQUIRED)
find_package(Jansson REQUIRED)
find_package(Libcurl REQUIRED)

include(cmake/ObsPluginHelpers.cmake)
find_package(obs-frontend-api REQUIRED)
find_qt(COMPONENTS Widgets Core)

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

configure_file(src/plugin-macros.h.in
               ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)
target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---

target_include_directories(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/obs
          ${LIBCURL_INCLUDE_DIRS} ${JANSSON_INCLUDE_DIRS})

target_link_libraries(
  ${CMAKE_PROJECT_NAME}
  PRIVATE OBS::libobs OBS::obs-frontend-api ${LIBCURL_LIBRARIES}
          ${JANSSON_LIBRARIES} Qt::Core Qt::Widgets)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  configure_file(cmake/bundle/windows/resource.rc.in
                 ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)
  target_sources(${CMAKE_PROJECT_NAME}
                 PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})

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

target_link_libraries(
  test PRIVATE OBS::libobs OBS::obs-frontend-api ${LIBCURL_LIBRARIES}
               ${JANSSON_LIBRARIES} Qt::Core Qt::Widgets)

set_target_properties(
  test
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_include_directories(
  test PRIVATE ${CMAKE_SOURCE_DIR}/src/obs ${CMAKE_SOURCE_DIR}/src
               ${LIBCURL_INCLUDE_DIRS} ${JANSSON_INCLUDE_DIRS})
