#include "nudgis-service.hpp"
#include "nudgis-config.hpp"
#include "plugin-macros.generated.h"

#include <string>
#include <vector>
#include <sstream>
#include <obs-module.h>

using namespace std;

#define NUDGIS_NAME          "Nudgis"

#define DEF_SERVER_URI       "rtmp"
#define DEF_STREAM_ID        "stream_id"
#define DEF_OID              "oid"

#define PATH_PREPARE_URL     "/api/v2/lives/prepare/"
#define PATH_START_URL       "/api/v2/lives/start/"
#define PATH_STOP_URL        "/api/v2/lives/stop/"

#define PARAM_API_KEY        "api_key="
#define PARAM_TITLE          "title="
#define PARAM_CHANNEL        "channel="
#define PARAM_OID            "oid="

bool GetRemoteFile(
    const char *url, std::string &str, std::string &error,
    long *responseCode = nullptr, const char *contentType = nullptr,
    std::string request_type = "", const char *postData = nullptr,
    std::vector<std::string> extraHeaders = std::vector<std::string>(),
    std::string *signature = nullptr, int timeoutSec = 0,
    bool fail_on_error = true);

static const string & GetRemoteFile(const string &url, const string &postData, bool * result = nullptr)
{
    static string response;
    string error = {};
    response = "";
    bool get_remote_file = GetRemoteFile(url.c_str(),response,error,nullptr,nullptr,"",postData.c_str());
    if (result != NULL)
        *result = get_remote_file;
    mlog(LOG_DEBUG, "GetRemoteFile (%s) response: %s", get_remote_file ? "OK" : "NOK",response.c_str());
    return response;
}


class NudgisData {
    private:
        NudgisConfig * nudgis_config =  NudgisConfig::GetCurrentNudgisConfig();

        bool GetResponseSuccess(obs_data_t * obs_data)
        {
            bool result = false;
            if (obs_data != NULL)
            {
                obs_data_set_default_bool(obs_data, "success", false);
                result = obs_data_get_string(obs_data, "success");
                mlog(LOG_DEBUG,"success: %s",result ? "TRUE" : "FALSE");
            }
            return result;
        }

        bool GetResponseSuccess(const string &response)
        {
            bool result = false;
            obs_data_t * obs_data = obs_data_create_from_json(response.c_str());
            if (obs_data != NULL)
            {
                result = this->GetResponseSuccess(obs_data);
                obs_data_release(obs_data);
            }
            return result;
        }

    public:
        string server_uri = DEF_SERVER_URI;
        string stream_id = DEF_STREAM_ID;
        string oid = DEF_OID;

        const string & PostData(const string &url, const string &postData, bool * result)
        {
            bool get_remote_file;
            const string &response = GetRemoteFile(url,postData,&get_remote_file);
            if (get_remote_file && result != NULL)
                *result = this->GetResponseSuccess(response);
            return response;
        }

        bool PostData(const string &url, const string &postData)
        {
            bool result;
            this->PostData(url,postData,&result);
            return result;
        }

        bool InitFromPrepareResponse(const string &prepare_response)
        {
            mlog(LOG_DEBUG, "Enter in %s", __func__);
            bool result = false;
            obs_data_t * obs_data = obs_data_create_from_json(prepare_response.c_str());
            if (obs_data != NULL)
            {
                obs_data_array_t * streams = obs_data_get_array(obs_data,"streams");
                if (streams != NULL)
                {
                    obs_data_t * stream = obs_data_array_item(streams, 0);
                    if (stream != NULL)
                    {
                        obs_data_set_default_string(stream, "server_uri", DEF_SERVER_URI);
                        this->server_uri = obs_data_get_string(stream, "server_uri");

                        obs_data_set_default_string(stream, "stream_id", DEF_STREAM_ID);
                        this->stream_id = obs_data_get_string(stream, "stream_id");

                        obs_data_release(stream);
                    }
                    obs_data_array_release(streams);
                }

                obs_data_set_default_string(obs_data, "oid", DEF_OID);
                this->oid = obs_data_get_string(obs_data, "oid");

                obs_data_set_default_bool(obs_data, "success", false);
                result = this->GetResponseSuccess(obs_data);

                obs_data_release(obs_data);
            }

            mlog(LOG_INFO,"success   : %s",result ? "TRUE" : "FALSE");
            mlog(LOG_INFO,"server_uri: %s",this->server_uri.c_str());
            mlog(LOG_INFO,"stream_id : %s",this->stream_id.c_str());
            mlog(LOG_INFO,"oid       : %s",this->oid.c_str());

            return result;
        }

        const string & GetPrepareUrl()
        {
            static string result;

            ostringstream prepare_url;
            prepare_url << this->nudgis_config->url << PATH_PREPARE_URL;
            result = prepare_url.str();
            mlog(LOG_DEBUG,"prepare_url: %s",result.c_str());

            return result;
        }

        const string & GetPreparePostdata()
        {
            static string result;

            ostringstream prepare_postdata;
            prepare_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_TITLE << this->nudgis_config->stream_title << "&" << PARAM_CHANNEL << this->nudgis_config->stream_channel;
            result = prepare_postdata.str();
            mlog(LOG_DEBUG,"prepare_postdata: %s",result.c_str());

            return result;
        }

        const string & GetStartUrl()
        {
            static string result;

            ostringstream start_url;
            start_url << this->nudgis_config->url << PATH_START_URL;
            result = start_url.str();
            mlog(LOG_DEBUG,"start_url: %s",result.c_str());

            return result;
        }

        const string & GetStartPostdata()
        {
            static string result;

            ostringstream start_postdata;
            start_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
            result = start_postdata.str();
            mlog(LOG_DEBUG,"start_postdata: %s",result.c_str());

            return result;
        }

        const string & GetStopUrl()
        {
            static string result;

            ostringstream stop_url;
            stop_url << this->nudgis_config->url << PATH_STOP_URL;
            result = stop_url.str();
            mlog(LOG_DEBUG,"stop_url: %s",result.c_str());

            return result;
        }

        const string & GetStopPostdata()
        {
            static string result;

            ostringstream stop_postdata;
            stop_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
            result = stop_postdata.str();
            mlog(LOG_DEBUG,"stop_postdata: %s",result.c_str());

            return result;
        }
};

static const char *nudgis_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    return obs_module_text(NUDGIS_NAME);
}

static void nudgis_destroy(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = (NudgisData *)data;
    delete nudgis_data;
}

static void *nudgis_create(obs_data_t *settings, obs_service_t *service)
{
    UNUSED_PARAMETER(settings);
    UNUSED_PARAMETER(service);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = new NudgisData();
    return nudgis_data;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    bool result = false;
    UNUSED_PARAMETER(output);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = (NudgisData *)data;

    string prepare_response = nudgis_data->PostData(nudgis_data->GetPrepareUrl(),nudgis_data->GetPreparePostdata(),&result);
    if (result)
    {
        nudgis_data->InitFromPrepareResponse(prepare_response);
        result = nudgis_data->PostData(nudgis_data->GetStartUrl(),nudgis_data->GetStartPostdata());
    }

    return result;
}

static const char *nudgis_url(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = (NudgisData *)data;
    return nudgis_data->server_uri.c_str();
}

static const char *nudgis_key(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = (NudgisData *)data;
    return nudgis_data->stream_id.c_str();
}

static void nudgis_deactivate(void *data)
{
    UNUSED_PARAMETER(data);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData * nudgis_data = (NudgisData *)data;
    nudgis_data->PostData(nudgis_data->GetStopUrl(),nudgis_data->GetStopPostdata());
}

static obs_properties_t *nudgis_properties(void *data)
{
    UNUSED_PARAMETER(data);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisConfig * nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    obs_property_t *p;
    obs_properties_t *ppts = obs_properties_create();
    p = obs_properties_add_list(ppts, "service", obs_module_text("Service"),
                    OBS_COMBO_TYPE_LIST,
                    OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string( p, nudgis_name(data), NUDGIS_NAME);

    p = obs_properties_add_list(ppts, "server", obs_module_text("Server"),
                OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    obs_property_list_add_string( p, obs_module_text(nudgis_config->url.c_str()), nudgis_config->url.c_str());

    return ppts;
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
    nudgis_deactivate, //void (*deactivate)(void *data);

    NULL,              //void (*update)(void *data, obs_data_t *settings);

    NULL,              //void (*get_defaults)(obs_data_t *settings);

    nudgis_properties, //obs_properties_t *(*get_properties)(void *data);

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
