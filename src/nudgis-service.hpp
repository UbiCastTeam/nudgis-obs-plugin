#ifndef NUDGIS_SERVICE_HPP
#define NUDGIS_SERVICE_HPP

extern struct obs_service_info nudgis_service_info;

void nudgis_upload_file(const char *filename, bool check_md5 = true);

#endif
