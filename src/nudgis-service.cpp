#include "nudgis-service.hpp"
#include "nudgis_data.h"

#include <string>
#include <vector>
#include <obs-module.h>
#include <curl/curl.h>
#include <jansson.h>

#define RTMP_PROTOCOL        "rtmp"

#define FMT_PREPARE_URL      "%s/api/v2/lives/prepare/"
#define FMT_START_URL        "%s/api/v2/lives/start/"


#define FMT_PREPARE_REQUEST  "api_key=%s&title=%s&channel=%s"
#define FMT_START_REQUEST    "api_key=%s&oid=%s"

extern bool GetRemoteFile(const char *url, std::string &str, std::string &error,
		   long *responseCode, const char *contentType,
		   std::string request_type, const char *postData,
		   std::vector<std::string> extraHeaders,
		   std::string *signature, int timeoutSec, bool fail_on_error);

typedef struct _buffer_data_t
{
    char * buffer;
    size_t len;
}buffer_data_t;

static size_t write_callback(void *data, size_t size, size_t nmemb, buffer_data_t * buffer_data)
{
    size_t size_new_data = size * nmemb;
    buffer_data->buffer = (char*)realloc(buffer_data->buffer,buffer_data->len + size_new_data);
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
	nudgis_t *service =  (nudgis_t *)data;

	bfree(service->server);
	bfree(service->key);

	service->server = bstrdup(RTMP_PROTOCOL);
	service->key = bstrdup("fake key");
}

static void nudgis_destroy(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	nudgis_t *service =  (nudgis_t *)data;

	bfree(service->server);
	bfree(service->key);
    bfree(service->oid);
	bfree(service);
}

static void *nudgis_create(obs_data_t *settings, obs_service_t *service)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	nudgis_t *data = (nudgis_t *)bzalloc(sizeof(struct nudgis));
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
    result = (char *)bmalloc(len_alloc);
    va_start (args, format);
    vsnprintf(result,len_alloc,format, args);
    va_end (args);
    return result;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    const nudgis_data_t * nudgis_data = get_nudgis_data();
    (void)output;
    nudgis_t * nudgis = (nudgis_t *)data;
    blog(LOG_INFO, "Enter in %s", __func__);
    buffer_data_t buffer_data = {};
    char * payload_prepare_url = bstrdup_printf(FMT_PREPARE_REQUEST,nudgis_data->apiKey,nudgis_data->streamTitle,nudgis_data->streamChannel);
    blog(LOG_INFO,"payload_prepare_url: %s",payload_prepare_url);
    char * prepare_url = bstrdup_printf(FMT_PREPARE_URL,nudgis_data->url);
    send_request(prepare_url,payload_prepare_url,&buffer_data);
    process_prepare_response(&buffer_data,nudgis);
    buffer_data_free(&buffer_data);

    char * payload_start_url = bstrdup_printf(FMT_START_REQUEST,nudgis_data->apiKey,nudgis->oid);
    blog(LOG_INFO,"payload_start_url: %s",payload_start_url);
    char * start_url = bstrdup_printf(FMT_START_URL,nudgis_data->url);
    send_request(start_url,payload_start_url,&buffer_data);
    blog(LOG_INFO,"start_request_response: %.*s",(int)buffer_data.len,buffer_data.buffer);

    bfree(payload_prepare_url);
    bfree(payload_start_url);
    bfree(prepare_url);
    bfree(start_url);

    return true;
}

static const char *nudgis_url(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	nudgis_t *service =  (nudgis_t *)data;
    return service->server;
}

static const char *nudgis_key(void *data)
{
    blog(LOG_INFO, "Enter in %s", __func__);
	nudgis_t *service =  (nudgis_t *)data;
    return service->key;
}

struct obs_service_info nudgis_service =
 {
	/* required */
	"nudgis",          // const char *id;

	nudgis_name,       //const char *(*get_name)(void *type_data);
	nudgis_create,     //void *(*create)(obs_data_t *settings, obs_service_t *service);
	nudgis_destroy,    //void (*destroy)(void *data);

	/* optional */
	NULL,              //void (*activate)(void *data, obs_data_t *settings);
	NULL,              //void (*deactivate)(void *data);

	nudgis_update,     //void (*update)(void *data, obs_data_t *settings);

	NULL,              //void (*get_defaults)(obs_data_t *settings);

	NULL,              //obs_properties_t *(*get_properties)(void *data);

	/**
	 * Called when getting ready to start up an output, before the encoders
	 * and output are initialized
	 *
	 * @param  data    Internal service data
	 * @param  output  Output context
	 * @return         true to allow the output to start up,
	 *                 false to prevent output from starting up
	 */
	nudgis_initialize, //bool (*initialize)(void *data, obs_output_t *output);

	nudgis_url,        //const char *(*get_url)(void *data);
	nudgis_key,        // const char *(*get_key)(void *data);

	NULL,              //const char *(*get_username)(void *data);
	NULL,              //const char *(*get_password)(void *data);

	NULL,              //bool (*deprecated_1)();

	NULL,              //void (*apply_encoder_settings)(void *data,
                       //       obs_data_t *video_encoder_settings,
                       //       obs_data_t *audio_encoder_settings);

	NULL,              //void *type_data;
	NULL,              //void (*free_type_data)(void *type_data);

	NULL,              //const char *(*get_output_type)(void *data);

	NULL,              //void (*get_supported_resolutions)(
                       //void *data, struct obs_service_resolution **resolutions,
                       //size_t *count);
	NULL,              //void (*get_max_fps)(void *data, int *fps);

	NULL,              //void (*get_max_bitrate)(void *data, int *video_bitrate,
                       //int *audio_bitrate);
};

void nudgis_service_register()
{
	obs_register_service(&nudgis_service);
}
