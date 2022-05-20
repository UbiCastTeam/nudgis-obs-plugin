#!/bin/bash

##############################################################################
# macOS libobs plugin package function
##############################################################################
#
# This script file can be included in build scripts for macOS or run directly
#
##############################################################################

# Halt on errors
set -eE

package_obs_plugin() {
    rm -rf ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}

    install -d ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/PlugIns
    install -d ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/Resources/data/obs-plugins/${PLUGIN_NAME}/locale

    install -m 0755 ${BUILD_FOLDER}/${PLUGIN_NAME}.so  ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/PlugIns/
    install -m 0644 ${BUILD_FOLDER}/../data/locale/* ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/Resources/data/obs-plugins/${PLUGIN_NAME}/locale/

    cat > ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/Resources/data/obs-plugins/${PLUGIN_NAME}/${PLUGIN_NAME}-uninstall.command <<EOF
#! /usr/bin/env bash
sudo rm -rf /Applications/OBS.app//Contents/PlugIns/${PLUGIN_NAME}.so /Applications/OBS.app//Contents/Resources/data/obs-plugins/${PLUGIN_NAME}
sudo pkgutil --forget ${PKGID}
EOF
    chmod a+x ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER}/Applications/OBS.app/Contents/Resources/data/obs-plugins/${PLUGIN_NAME}/${PLUGIN_NAME}-uninstall.command

    pkgbuild --identifier ${PKGID} --root ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER} ${BUILD_FOLDER}/${PLUGIN_NAME}-macos-${ARCH}_${PLUGIN_VERSION}.pkg
}

package-plugin-standalone() {
    CHECKOUT_DIR="$(/usr/bin/git rev-parse --show-toplevel)"
    if [ -f "${CHECKOUT_DIR}/CI/include/build_environment.sh" ]; then
        source "${CHECKOUT_DIR}/CI/include/build_environment.sh"
    fi
    PLUGIN_NAME="${PRODUCT_NAME:-nudgis-obs-plugin}"
    BUILD_FOLDER="${BUILD_FOLDER:-build}"
    BUNDLE_IMAGE_FOLDER="${BUNDLE_IMAGE_FOLDER:-bundle-image}"
    PKGID="${PKGID:-eu.ubicast.nudgis-obs-plugin.pkg}"
    ARCH="${ARCH:-x86_64}"
    PLUGIN_VERSION="${PLUGIN_VERSION:-27.2.4_1.0.0}"

    package_obs_plugin
}

print_usage() {
    echo -e "Usage: ${0}\n" \
            "-h, --help                     : Print this help\n" \
            "-q, --quiet                    : Suppress most build process output\n" \
            "-v, --verbose                  : Enable more verbose build process output\n" \
            "-a, --architecture             : Specify build architecture (default: x86_64, alternative: arm64)\n" \
            "--build-dir                    : Specify alternative build directory (default: build)\n" \
            "--bundle-image-folder          : Specify alternative bundle-image directory (default: bundle-image)\n" \
            "--pkgid                        : Specify alternative pkgid (default: eu.ubicast.nudgis-obs-plugin.pkg)\n"
}

package-plugin-main() {
    if [ -z "${_RUN_OBS_BUILD_SCRIPT}" ]; then
        while true; do
            case "${1}" in
                -h | --help ) print_usage; exit 0 ;;
                -q | --quiet ) export QUIET=TRUE; shift ;;
                -v | --verbose ) export VERBOSE=TRUE; shift ;;
                -a | --architecture ) ARCH="${2}"; shift 2 ;;
                --build-dir ) BUILD_FOLDER="${2}"; shift 2 ;;
                --bundle-image-folder ) BUNDLE_IMAGE_FOLDER="${2}"; shift 2 ;;
                --pkgid ) PKGID="${2}"; shift 2 ;;
                --plugin-version ) PLUGIN_VERSION="${2}"; shift 2 ;;
                -- ) shift; break ;;
                * ) break ;;
            esac
        done

        package-plugin-standalone
    fi
}

package-plugin-main $*
