#include <obs-module.h>
#include "nudgis-service.h"
#include <curl/curl.h>
#include <jansson.h>
#include <unistd.h>

#define FAKE_API_KEY ""
#define FAKE_TITLE "MonStream2"
#define FAKE_CHANNEL "MonChannel"

#define PREPARE_URL  "https://beta.ubicast.net/api/v2/lives/prepare/"
#define START_URL  "https://beta.ubicast.net/api/v2/lives/start/"

#define FMT_PREPARE_REQUEST  "api_key=%s&title=%s&channel=%s"
#define FMT_START_REQUEST  "api_key=%s&oid=%s"

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

typedef struct nudgis {
	char *server, *key, *oid;
} nudgis_t;

static void buffer_data_free(buffer_data_t * buffer_data)
{
    if (buffer_data != NULL)
    {
        free(buffer_data->buffer);
        buffer_data->buffer = NULL;
        buffer_data->len = 0;
    }
}

void process_prepare_response(const buffer_data_t * buffer_data,nudgis_t * nudgis)
{
    if (buffer_data != NULL && nudgis != NULL)
    {

        if (buffer_data->buffer != NULL)
        {        
            json_t *root;        
            root = json_loadb(buffer_data->buffer,buffer_data->len,0,NULL);
            json_t *streams = json_object_get(root,"streams");
            json_t *oid = json_object_get(root,"oid");
            const char * oid_str = json_string_value(oid);
            json_t *stream= json_array_get(streams,0);
            json_t *server_uri = json_object_get(stream,"server_uri");
            const char * server_uri_str = json_string_value(server_uri);
            json_t *stream_id = json_object_get(stream,"stream_id");
            const char * stream_id_str = json_string_value(stream_id);

            blog(LOG_INFO,"server_uri_str: %s",server_uri_str);
            blog(LOG_INFO,"stream_id_str: %s",stream_id_str);

            bfree(nudgis->key);
            nudgis->key = bstrdup(stream_id_str);

            bfree(nudgis->server);
            nudgis->server = bstrdup(server_uri_str);

            bfree(nudgis->oid);
            nudgis->oid = bstrdup(oid_str);

            json_decref(root);
        }
    }
}

int send_request(const char * url,char * data,buffer_data_t * buffer_data)
{
    int result = -1;
    if (buffer_data != NULL && url != NULL)
    {
        CURL *curl;
        buffer_data_free(buffer_data);
        CURLcode res;
        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();

        if(curl) 
        {

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        if (buffer_data->buffer != NULL)
        {
            printf("%.*s\n",(int)buffer_data->len,buffer_data->buffer);
            result = 0;
        }
    }
    return result;
}

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
    bfree(service->oid);
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

static char * bstrdup_printf(const char * format, ... )
{
    char * result = NULL;
    va_list args;
    va_start (args, format);
    int len_alloc = vsnprintf(NULL,0,format, args) + 1;
    result = bzalloc(len_alloc);
    va_start (args, format);
    vsnprintf(result,len_alloc,format, args);
    va_end (args);
    return result;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    (void)output;
    nudgis_t * nudgis = data;
    blog(LOG_INFO, "Enter in %s", __func__);
    buffer_data_t buffer_data = {0};
    char * payload_prepare_url = bstrdup_printf(FMT_PREPARE_REQUEST,FAKE_API_KEY,FAKE_TITLE,FAKE_CHANNEL);
    blog(LOG_INFO,"payload_prepare_url: %s",payload_prepare_url);
    send_request(PREPARE_URL,payload_prepare_url,&buffer_data);
    process_prepare_response(&buffer_data,nudgis);
    buffer_data_free(&buffer_data);

    char * payload_start_url = bstrdup_printf(FMT_START_REQUEST,FAKE_API_KEY,nudgis->oid);
    blog(LOG_INFO,"payload_start_url: %s",payload_start_url);
    send_request(START_URL,payload_start_url,&buffer_data);
    blog(LOG_INFO,"start_request_response: %.*s",(int)buffer_data.len,buffer_data.buffer);

    bfree(payload_prepare_url);
    bfree(payload_start_url);

    return true;
}

static const char *nudgis_url(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;
    return service->server;
}

static const char *nudgis_key(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	struct nudgis *service =  data;
    return service->key;
}


struct obs_service_info nudgis_service = 
{
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
