#include <obs-module.h>
#include "nudgis-service.h"

struct nudgis {
	char *server, *key;
};

static const char *nudgis_name(void *unused)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	UNUSED_PARAMETER(unused);
	return obs_module_text("Nudgis");
}

static void nudgis_update(void *data, obs_data_t *settings)
{
    (void)settings;
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;

	bfree(service->server);
	bfree(service->key);

	//service->server = bstrdup(obs_data_get_string(settings, "server"));
	//service->key = bstrdup(obs_data_get_string(settings, "key"));
	service->server = bstrdup("fake server");
	service->key = bstrdup("fake key");
}

static void nudgis_destroy(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;

	bfree(service->server);
	bfree(service->key);
	bfree(service);
}

static void *nudgis_create(obs_data_t *settings, obs_service_t *service)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *data = bzalloc(sizeof(struct nudgis));
	nudgis_update(data, settings);

	UNUSED_PARAMETER(service);
	return data;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    (void)output;
    (void)data;
    blog(LOG_INFO, "Enter in %s", __func__);
    return true;
}

static const char *nudgis_url(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;
    (void)service;
	return "mystreamurl";
}

static const char *nudgis_key(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;
    (void)service;
	return "mystreamkey";
}


struct obs_service_info nudgis_service = {
	.id = "nudgis",
	.get_name = nudgis_name,
	.create = nudgis_create,
	.destroy = nudgis_destroy,
	.update = nudgis_update,
	.initialize = nudgis_initialize,
	.get_url = nudgis_url,
	.get_key = nudgis_key,
};

void nudgis_service_register()
{
	obs_register_service(&nudgis_service);
}