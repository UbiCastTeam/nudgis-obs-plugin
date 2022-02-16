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

    tar -C ${BUILD_FOLDER}/${BUNDLE_IMAGE_FOLDER} -caf ${BUILD_FOLDER}/${PLUGIN_NAME}.tar.gz Applications/OBS.app/Contents/PlugIns/ Applications/OBS.app/Contents/Resources/data/obs-plugins/${PLUGIN_NAME}/locale

    cat > ${BUILD_FOLDER}/${PLUGIN_NAME}-install.command <<EOF
#! /usr/bin/env bash
cd \$(dirname "\$0")
tar -C / -xvf ${PLUGIN_NAME}.tar.gz
EOF
    chmod a+x ${BUILD_FOLDER}/${PLUGIN_NAME}-install.command

    cat > ${BUILD_FOLDER}/${PLUGIN_NAME}-uninstall.command <<EOF
#! /usr/bin/env bash
rm -rf /Applications/OBS.app//Contents/PlugIns/${PLUGIN_NAME}.so /Applications/OBS.app//Contents/Resources/data/obs-plugins/${PLUGIN_NAME}
EOF
    chmod a+x ${BUILD_FOLDER}/${PLUGIN_NAME}-uninstall.command

    tar -C ${BUILD_FOLDER} -caf ${BUILD_FOLDER}/${PLUGIN_NAME}-package.tar.gz ${PLUGIN_NAME}.tar.gz ${PLUGIN_NAME}-uninstall.command ${PLUGIN_NAME}-install.command
}

package-plugin-standalone() {
    CHECKOUT_DIR="$(/usr/bin/git rev-parse --show-toplevel)"
    if [ -f "${CHECKOUT_DIR}/CI/include/build_environment.sh" ]; then
        source "${CHECKOUT_DIR}/CI/include/build_environment.sh"
    fi
    PLUGIN_NAME="${PRODUCT_NAME:-nudgis-obs-plugin}"
    BUILD_FOLDER="${BUILD_FOLDER:-build}"
    BUNDLE_IMAGE_FOLDER="${BUNDLE_IMAGE_FOLDER:-bundle-image}"

    package_obs_plugin
}

print_usage() {
    echo -e "Usage: ${0}\n" \
            "-h, --help                     : Print this help\n" \
            "-q, --quiet                    : Suppress most build process output\n" \
            "-v, --verbose                  : Enable more verbose build process output\n" \
            "--build-dir                    : Specify alternative build directory (default: build)\n" \
            "--bundle-image-folder          : Specify alternative bundle-image directory (default: bundle-image)\n"
}

package-plugin-main() {
    if [ -z "${_RUN_OBS_BUILD_SCRIPT}" ]; then
        while true; do
            case "${1}" in
                -h | --help ) print_usage; exit 0 ;;
                -q | --quiet ) export QUIET=TRUE; shift ;;
                -v | --verbose ) export VERBOSE=TRUE; shift ;;
                --build-dir ) BUILD_FOLDER="${2}"; shift 2 ;;
                --bundle-image-folder ) BUNDLE_IMAGE_FOLDER="${2}"; shift 2 ;;
                -- ) shift; break ;;
                * ) break ;;
            esac
        done

        package-plugin-standalone
    fi
}

package-plugin-main $*
