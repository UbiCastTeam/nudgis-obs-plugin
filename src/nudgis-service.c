#include <obs-module.h>
#include "nudgis-service.h"
#include <curl/curl.h>

#define DEFAULT_DATA "api_key=no_key&title=MonStream2"

typedef struct _buffer_data_t
{
    char * buffer;
    size_t len;
}buffer_data_t;

static size_t write_callback(void *data, size_t size, size_t nmemb, buffer_data_t * buffer_data)
{
    size_t size_new_data = size * nmemb;
    buffer_data->buffer = realloc(buffer_data->buffer,buffer_data->len + size_new_data);
    if (buffer_data->buffer)
    {
        memcpy(buffer_data->buffer + buffer_data->len,data,size_new_data);
        buffer_data->len += size_new_data;
    }
    return size_new_data;
}

int liveprepare(char * data)
{
    FILE * prepjson;
    CURL *curl;
    buffer_data_t buffer_data = {.buffer = NULL, .len = 0};
    CURLcode res;
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) 
    {

    curl_easy_setopt(curl, CURLOPT_URL, "https://beta.ubicast.net/api/v2/lives/prepare/");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    res = curl_easy_perform(curl);

    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    if (buffer_data.buffer != NULL)
    {
        printf("%.*s\n",buffer_data.len,buffer_data.buffer);
        free(buffer_data.buffer);
    }

    return 0;
}

struct nudgis {
	char *server, *key;
};

static const char *nudgis_name(void *unused)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	UNUSED_PARAMETER(unused);
	return obs_module_text("Nudgis");
}

static void nudgis_update(void *data, obs_data_t *settings)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
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
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	struct nudgis *service =  data;

	bfree(service->server);
	bfree(service->key);
	bfree(service);
}

static void *nudgis_create(obs_data_t *settings, obs_service_t *service)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	struct nudgis *data = bzalloc(sizeof(struct nudgis));
	nudgis_update(data, settings);

	UNUSED_PARAMETER(service);
	return data;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	char * httpData = DEFAULT_DATA;
	printf("start with: '%s'\n",data);
    liveprepare(httpData);
    return true;
}

static obs_properties_t *nudgis_properties(void *unused)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	UNUSED_PARAMETER(unused);

	obs_properties_t *ppts = obs_properties_create();
	obs_property_t *p;

	obs_properties_add_text(ppts, "server", "URL", OBS_TEXT_DEFAULT);

	obs_properties_add_text(ppts, "key", obs_module_text("StreamKey"),
				OBS_TEXT_PASSWORD);

	return ppts;
}


static const char *nudgis_url(void *data)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	struct nudgis *service =  data;
	return "rtmp://beta.ubicast.net/FE6ZrfWAUCuRCJ9xAFHZfqzeX86EA5";
}

static const char *nudgis_key(void *data)
{
    blog(LOG_INFO, "Enter in %s", __FUNCTION__);
	struct nudgis *service =  data;
	return "beta_msuser_H8TLhntzEYGqO0g9n7vYCoXs4wQ1l6_720";
}


struct obs_service_info nudgis_service = {
	.id = "nudgis",
	.get_name = nudgis_name,
	.create = nudgis_create,
	.destroy = nudgis_destroy,
	.update = nudgis_update,
	.initialize = nudgis_initialize,
	.get_properties = nudgis_properties,
	.get_url = nudgis_url,
	.get_key = nudgis_key,
};

void nudgis_service_register()
{
	obs_register_service(&nudgis_service);
}