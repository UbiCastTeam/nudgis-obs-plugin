#ifndef NUDGIS_SERVICE_HPP
#define NUDGIS_SERVICE_HPP

#include <cstddef>

extern struct obs_service_info nudgis_service_info;

typedef void (*NudgisUploadProgressCb)(void *cb_args,int percent);

void nudgis_upload_file(const char *filename,NudgisUploadProgressCb nudgis_upload_progress_cb = NULL, void *cb_args = NULL, bool check_md5 = true);

#endif
