#ifndef NUDGIS_DATA_H
#define NUDGIS_DATA_H

typedef struct _nudgis_data
{
    char * apiKey;
    char * url;
    char * streamTitle;
    char * streamChannel;
    char * vodTitle;
    char * vodChannel;
}nudgis_data_t;

void set_nudgis_data(const nudgis_data_t * nudgis_data);
const nudgis_data_t * get_nudgis_data();
void free_nudgis_data();

#endif