# Once done these will be defined:
#
#  LIBOBSFRONTENDAPI_FOUND
#  LIBOBSFRONTENDAPI_INCLUDE_DIR
#  LIBOBSFRONTENDAPI_LIBRARIES

find_path(LIBOBSFRONTENDAPI_INCLUDE_DIR
	NAMES obs-frontend-api.h
	HINTS
		ENV LibObs_DIR
		ENV LibObsFrontendApi_DIR
		${LibObs_DIR}
		${LibObsFrontendApi_DIR}
	PATHS
		/usr/include /usr/local/include /opt/local/include /sw/include
	PATH_SUFFIXES
		../../UI/obs-frontend-api
		obs)

find_library(LIBOBSFRONTENDAPI_LIBRARIES
	NAMES obs-frontend-api libobs-frontend-api
	HINTS
		ENV LibObs_DIR
		ENV LibObsFrontendApi_DIR
		${LibObs_DIR}
		${LibObsFrontendApi_DIR}
	PATHS
		/usr/lib /usr/local/lib /opt/local/lib /sw/lib
	PATH_SUFFIXES
		../UI/obs-frontend-api/RelWithDebInfo
		../UI/obs-frontend-api)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibObsFrontendApi DEFAULT_MSG LIBOBSFRONTENDAPI_LIBRARIES LIBOBSFRONTENDAPI_INCLUDE_DIR)
mark_as_advanced(LIBOBSFRONTENDAPI_INCLUDE_DIR LIBOBSFRONTENDAPI_LIBRARIES)
