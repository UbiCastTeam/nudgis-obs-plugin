#include "nudgis_data.h"

#include <stddef.h>
#include <obs.h>

static nudgis_data_t local_nudgis_data = {0};

void free_nudgis_data()
{
    bfree(local_nudgis_data.apiKey);
    bfree(local_nudgis_data.streamChannel);
    bfree(local_nudgis_data.streamTitle);
    bfree(local_nudgis_data.url);
    bfree(local_nudgis_data.vodChannel);
    bfree(local_nudgis_data.vodTitle);
}

void set_nudgis_data(const nudgis_data_t * nudgis_data)
{
    if (nudgis_data != NULL)
    {
        free_nudgis_data();
        local_nudgis_data = *nudgis_data;
    }
}

const nudgis_data_t * get_nudgis_data()
{
    return &local_nudgis_data;
}
