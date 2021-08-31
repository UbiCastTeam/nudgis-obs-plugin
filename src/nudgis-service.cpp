#include "nudgis-service.hpp"
#include "nudgis_data.h"

#include <string>
#include <vector>
#include <sstream>
#include <obs-module.h>
#include <jansson.h>

using namespace std;

#define RTMP_PROTOCOL        "rtmp"

#define PATH_PREPARE_URL     "/api/v2/lives/prepare/"
#define PATH_START_URL       "/api/v2/lives/start/"

#define PARAM_API_KEY        "api_key="
#define PARAM_TITLE          "title="
#define PARAM_CHANNEL        "channel="
#define PARAM_OID            "oid="

typedef struct nudgis {
	char *server, *key, *oid;
} nudgis_t;

void process_prepare_response(string response,nudgis_t * nudgis)
{
    if (nudgis != NULL)
    {
        json_t *root;
        root = json_loads(response.c_str(),0,NULL);
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

bool GetRemoteFile(
	const char *url, std::string &str, std::string &error,
	long *responseCode = nullptr, const char *contentType = nullptr,
	std::string request_type = "", const char *postData = nullptr,
	std::vector<std::string> extraHeaders = std::vector<std::string>(),
	std::string *signature = nullptr, int timeoutSec = 0,
	bool fail_on_error = true);

bool GetRemoteFile(const char *url, std::string &str, const char *postData = nullptr)
{
    bool result = false;
    string error = {};
    result =  GetRemoteFile(url,str,error,nullptr,nullptr,"",postData);
    blog(LOG_DEBUG, "GetRemoteFile response: %s", str.c_str());
    return result;
}

bool GetRemoteFile(const char *url, const char *postData = nullptr)
{
    string str = {};
    return GetRemoteFile(url, str, postData);
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    blog(LOG_INFO, "Enter in %s", __func__);
    (void)output;
    const nudgis_data_t * nudgis_data = get_nudgis_data();
    nudgis_t * nudgis = (nudgis_t *)data;

    ostringstream prepare_url("");
    prepare_url << nudgis_data->url << PATH_PREPARE_URL;
    blog(LOG_INFO,"prepare_url: %s",prepare_url.str().c_str());

    ostringstream prepare_postdata("");
    prepare_postdata << PARAM_API_KEY << nudgis_data->apiKey << "&" << PARAM_TITLE << nudgis_data->streamTitle << "&" << PARAM_CHANNEL << nudgis_data->streamChannel;
    blog(LOG_INFO,"prepare_postdata: %s",prepare_postdata.str().c_str());

    string response_url = {};
    GetRemoteFile(prepare_url.str().c_str(),response_url,prepare_postdata.str().c_str());

    process_prepare_response(response_url,nudgis);

    ostringstream start_url("");
    start_url << nudgis_data->url << PATH_START_URL;
    blog(LOG_INFO,"start_url: %s",start_url.str().c_str());

    ostringstream start_postdata("");
    start_postdata << PARAM_API_KEY << nudgis_data->apiKey << "&" << PARAM_OID << nudgis->oid;
    blog(LOG_INFO,"start_postdata: %s",start_postdata.str().c_str());

    GetRemoteFile(start_url.str().c_str(),start_postdata.str().c_str());

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
