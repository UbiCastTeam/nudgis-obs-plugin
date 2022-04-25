#ifndef NUDGIS_SERVICE_HPP
#define NUDGIS_SERVICE_HPP

#include <cstddef>
#include <obs.h>

extern struct obs_service_info nudgis_service_info;

typedef void (*NudgisUploadProgressCb)(void *cb_args,int percent);

class NudgisUploadFileResult
{
    public:
        ~NudgisUploadFileResult();
        obs_data_t * upload_complete_response = NULL;
        obs_data_t * media_add_response = NULL;
};

NudgisUploadFileResult *nudgis_upload_file(const char *filename,NudgisUploadProgressCb nudgis_upload_progress_cb = NULL, void *cb_args = NULL, bool check_md5 = true);

#endif
