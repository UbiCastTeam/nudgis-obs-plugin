cmake_minimum_required(VERSION 3.16)

# Change nudgis-obs-plugin to your plugin's name in a machine-readable format
# (e.g.: obs-myawesomeplugin) and set 
project(nudgis-obs-plugin VERSION 1.0.0)
add_library(${CMAKE_PROJECT_NAME} MODULE)

add_executable(test)

target_sources(test
	PRIVATE
		src/nudgis-plugin.cpp
		src/nudgis-service.cpp
		src/nudgis-config.cpp
		src/nudgis-upload-ui.cpp
		src/http-client.cpp
		src/obs/obs-utils.cpp
		src/test.cpp)

# Replace `Your Name Here` with the name (yours or your organization's) you want
# to see as the author of the plugin (in the plugin's metadata itself and in the installers)
set(PLUGIN_AUTHOR "Ubicast SA")

# Replace `com.example.obs-plugin-template` with a unique Bundle ID for macOS releases
# (used both in the installer and when submitting the installer for notarization)
set(MACOS_BUNDLEID "com.ubicast.nudgis-obs-plugin")

# Replace `me@contoso.com` with the maintainer email address you want to put in Linux packages
set(LINUX_MAINTAINER_EMAIL "noreply@ubicast.eu")

# Add your custom source files here - header files are optional and only required for visibility
# e.g. in Xcode or Visual Studio
target_sources(${CMAKE_PROJECT_NAME}
	PRIVATE
		src/nudgis-plugin.cpp
		src/nudgis-service.cpp
		src/nudgis-config.cpp
		src/nudgis-upload-ui.cpp
		src/http-client.cpp
		src/obs/obs-utils.cpp)

# /!\ TAKE NOTE: No need to edit things past this point /!\

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(Jansson REQUIRED)
find_package(Libcurl REQUIRED)
find_package(LibObs REQUIRED)
find_package(LibObsFrontendApi REQUIRED)
include(external/ObsPluginHelpers.cmake)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)


configure_file(
	src/plugin-macros.h.in
	${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME}
	PRIVATE
		src/plugin-macros.generated.h)

# --- Platform-independent build settings ---

target_include_directories(test
	PRIVATE
		${CMAKE_SOURCE_DIR}/src/obs
		${CMAKE_SOURCE_DIR}/src
		${LIBOBSFRONTENDAPI_INCLUDE_DIR}
		${LIBCURL_INCLUDE_DIRS}
		${JANSSON_INCLUDE_DIRS})

target_link_libraries(test
	PRIVATE
		libobs
		Qt5::Core
		Qt5::Widgets
		${LIBOBSFRONTENDAPI_LIBRARIES}
		${LIBCURL_LIBRARIES}
		${JANSSON_LIBRARIES})

set_target_properties(test PROPERTIES
	AUTOMOC ON
	AUTOUIC ON
	AUTORCC ON)

target_include_directories(${CMAKE_PROJECT_NAME}
	PRIVATE
		${CMAKE_SOURCE_DIR}/src/obs
		${CMAKE_SOURCE_DIR}/src
		${LIBOBSFRONTENDAPI_INCLUDE_DIR}
		${LIBCURL_INCLUDE_DIRS}
		${JANSSON_INCLUDE_DIRS})

target_link_libraries(${CMAKE_PROJECT_NAME}
	PRIVATE
		libobs
		Qt5::Core
		Qt5::Widgets
		${LIBOBSFRONTENDAPI_LIBRARIES}
		${LIBCURL_LIBRARIES}
		${JANSSON_LIBRARIES})

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
	AUTOMOC ON
	AUTOUIC ON
	AUTORCC ON)

target_compile_features(${CMAKE_PROJECT_NAME}
	PRIVATE
		cxx_std_17)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
	configure_file(
		installer/installer-Windows.iss.in
		${CMAKE_SOURCE_DIR}/installer/installer-Windows.generated.iss)

	configure_file(
		CI/include/build_environment.ps1.in
		${CMAKE_SOURCE_DIR}/CI/include/build_environment.ps1)

	if(MSVC)
		target_compile_options(${CMAKE_PROJECT_NAME}
			PRIVATE
				/MP
				/d2FH4-)
	endif()
# --- End of section ---

# -- macOS specific build settings and tasks --
elseif(OS_MACOS)
	configure_file(
		bundle/installer-macOS.pkgproj.in
		${CMAKE_SOURCE_DIR}/bundle/installer-macOS.generated.pkgproj)

	configure_file(
		CI/include/build_environment.sh.in
		${CMAKE_SOURCE_DIR}/CI/include/build_environment.sh
	)

	set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
	set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
	set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

	target_compile_options(${CMAKE_PROJECT_NAME}
		PRIVATE
			-Wall
			-Wextra
			-Werror-implicit-function-declaration
			-stdlib=libc++
			-fvisibility=default)

	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
# --- End of section ---

# --- Linux-specific build settings and tasks ---
else()
	configure_file(
		CI/include/build_environment.sh.in
		${CMAKE_SOURCE_DIR}/CI/include/build_environment.sh
	)

	target_compile_options(${CMAKE_PROJECT_NAME}
		PRIVATE
			-Wall
			-Wextra)

	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES PREFIX "")
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})
