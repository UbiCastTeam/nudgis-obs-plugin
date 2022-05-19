#include "nudgis-service.hpp"
#include "plugin-macros.generated.h"

#include <sstream>
#include <obs-module.h>
#include <jansson.h>
#include <obs-frontend-api.h>
#include <fstream>
#include <cmath>

#define NUDGIS_NAME "Nudgis"
#define ORIGIN "nudgis-obs-plugin"

#define DEF_MULTI_STREAMS "no"
#define DEF_KEYINT_SEC 3
#define DEF_VERSION_NUMBER "6.5.4"

#define PATH_PREPARE_URL "/api/v2/lives/prepare/"
#define PATH_START_URL "/api/v2/lives/start/"
#define PATH_STOP_URL "/api/v2/lives/stop/"
#define PATH_API_BASE_URL "/api/v2/"
#define PATH_UPLOAD_URL "upload/"
#define PATH_UPLOADCOMPLETE_URL "upload/complete/"
#define PATH_MEDIASADD_URL "medias/add/"
#define PATH_CHANNELS_PERSONAL_URL "/api/v2/channels/personal/"
#define PATH_MEDIAS_DELETE_URL "/api/v2/medias/delete/"

#define PARAM_API_KEY "api_key="
#define PARAM_TITLE "title="
#define PARAM_CHANNEL "channel="
#define PARAM_OID "oid="
#define PARAM_MULTI_STREAMS "multi_streams="
#define PARAM_STREAMS "streams="
#define PARAM_UPLOAD_ID "upload_id="
#define PARAM_MD5 "md5="
#define PARAM_NO_MD5 "no_md5="
#define PARAM_ORIGIN "origin="
#define PARAM_CODE "code="
#define PARAM_DELETE_METADATA "delete_metadata="
#define PARAM_DELETE_RESOURCES "delete_resources="

#define FILENAME_STREAMENCODER "streamEncoder.json"

#define PREFIX_TEST_STREAM_TITLE "Test Live - "

static const char *NUDGIS_UPLOAD_STATE_STR[] =
        {
                "IDLE",               // [NudgisUpload::NUDGIS_UPLOAD_STATE_IDLE]
                "UPLOAD_IN_PROGRESS", // [NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_IN_PROGRESS]
                "UPLOAD_SUCESSFULL",  // [NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL]
                "UPLOAD_CANCEL",      // [NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_CANCEL]
                "UPLOAD_FAILED",      // [NudgisUpload::NUDGIS_UPLOAD_STATE_UPLOAD_FAILED]
};

static const char *NUDGISDATA_LIVE_TEST_RESULT_STR[] =
        {
                "SUCESS", // [NudgisData::NUDGISDATA_LIVE_TEST_RESULT_SUCESS]
                "FAILED", // [NudgisData::NUDGISDATA_LIVE_TEST_RESULT_FAILED]
};

#define DEF_WIDTH 800
#define DEF_HEIGHT 600
#define DEF_VIDEO_BITRATE 1500
#define DEF_AUDIO_BITRATE 64
#define DEF_FRAMERATE 30

NudgisStreams::NudgisStreams()
        : NudgisStreams(DEF_WIDTH, DEF_HEIGHT, DEF_VIDEO_BITRATE, DEF_AUDIO_BITRATE, DEF_FRAMERATE)
{
}

NudgisStreams::NudgisStreams(int width, int height, int video_bitrate, int audio_bitrate, int framerate)
{
    this->width = width;
    this->height = height;
    this->video_bitrate = video_bitrate;
    this->audio_bitrate = audio_bitrate;
    this->framerate = framerate;
}

NudgisStreams::NudgisStreams(obs_output_t *output)
{
    if (output != NULL) {
        obs_encoder_t *venc = obs_output_get_video_encoder(output);
        obs_encoder_t *aenc = obs_output_get_audio_encoder(output, 0);
        obs_data_t *vsettings = obs_encoder_get_settings(venc);
        obs_data_t *asettings = obs_encoder_get_settings(aenc);

        this->video_bitrate = obs_data_get_int(vsettings, "bitrate") * 1000;
        this->audio_bitrate = obs_data_get_int(asettings, "bitrate") * 1000;

        obs_data_release(vsettings);
        obs_data_release(asettings);

        const struct video_output_info *output_video_info = video_output_get_info(obs_output_video(output));
        this->width = output_video_info->width;
        this->height = output_video_info->height;
        this->framerate = output_video_info->fps_num;
    }
}

const std::string &NudgisStreams::GetJson() const
{
    static std::string result;

    json_t *streams = json_array();
    if (streams != NULL) {
        json_t *stream = json_object();
        if (stream != NULL) {
            json_object_set(stream, "width", json_integer(this->width));
            json_object_set(stream, "height", json_integer(this->height));
            json_object_set(stream, "video_bitrate", json_integer(this->video_bitrate));
            json_object_set(stream, "audio_bitrate", json_integer(this->audio_bitrate));
            json_object_set(stream, "framerate", json_integer(this->framerate));

            json_array_append_new(streams, stream);
        }

        result = json_dumps(streams, 0);
        json_decref(streams);
    }

    return result;
}

HttpClient &NudgisData::GetHttpClient()
{
    return this->http_client;
}

bool NudgisData::GetResponseSuccess(obs_data_t *obs_data)
{
    bool result = false;
    if (obs_data != NULL) {
        obs_data_set_default_bool(obs_data, "success", false);
        result = obs_data_get_string(obs_data, "success");
        mlog(LOG_DEBUG, "success: %s", result ? "TRUE" : "FALSE");
    }
    return result;
}

bool NudgisData::GetResponseSuccess(const std::string &response)
{
    bool result = false;
    obs_data_t *obs_data = obs_data_create_from_json(response.c_str());
    if (obs_data != NULL) {
        result = this->GetResponseSuccess(obs_data);
        obs_data_release(obs_data);
    }
    return result;
}

NudgisData::NudgisData(NudgisConfig *nudgis_config)
        : NudgisData(NULL, nudgis_config) {}

NudgisData::NudgisData(obs_data_t *settings, NudgisConfig *nudgis_config)
{
    this->nudgis_config = nudgis_config;
    if (this->nudgis_config == NULL) {
        this->nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    }

    this->settings = settings;
}

NudgisData::~NudgisData()
{
    if (server_version != NULL)
        delete server_version;
    if (url_prefix != NULL)
        delete url_prefix;
}

const std::string &NudgisData::GetUrlPrefix()
{
    if (this->url_prefix == NULL) {
        this->url_prefix = new std::string(*this->GetServerVersion() < QVersionNumber(8, 2) ? "medias/resource/" : "");
    }
    return *this->url_prefix;
}

const std::string &NudgisData::GetData(const std::string &url, const std::string &getData, bool *result)
{
    bool send_result;

    this->http_client.setUrl(url);
    this->http_client.setParameters(getData);
    this->http_client.setMethod(HttpClient::HTTP_CLIENT_METHOD_GET);
    send_result = this->http_client.send();
    if (result != NULL)
        *result = send_result;

    return this->http_client.getResponse();
}

const std::string &NudgisData::PostData(const std::string &url, const std::string &postData, bool *result)
{
    bool send_result;

    this->http_client.setUrl(url);
    this->http_client.setParameters(postData);
    this->http_client.setMethod(HttpClient::HTTP_CLIENT_METHOD_POST);
    send_result = this->http_client.send();
    if (result != NULL)
        *result = send_result;

    return this->http_client.getResponse();
}

bool NudgisData::PostData(const std::string &url, const std::string &postData)
{
    bool result;
    this->PostData(url, postData, &result);
    return result;
}

bool NudgisData::InitFromPrepareResponse(const std::string &prepare_response)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    bool result = false;
    obs_data_t *obs_data = obs_data_create_from_json(prepare_response.c_str());
    if (obs_data != NULL) {
        obs_data_array_t *streams = obs_data_get_array(obs_data, "streams");
        if (streams != NULL) {
            obs_data_t *stream = obs_data_array_item(streams, 0);
            if (stream != NULL) {
                obs_data_set_default_string(stream, "server_uri", DEF_SERVER_URI);
                this->server_uri = obs_data_get_string(stream, "server_uri");

                obs_data_set_default_string(stream, "stream_id", DEF_STREAM_ID);
                this->stream_id = obs_data_get_string(stream, "stream_id");

                obs_data_set_string(settings, "key", this->stream_id.c_str());
                obs_frontend_save_streaming_service();

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

    mlog(LOG_INFO, "success   : %s", result ? "TRUE" : "FALSE");
    mlog(LOG_INFO, "server_uri: %s", this->server_uri.c_str());
    mlog(LOG_INFO, "stream_id : %s", this->stream_id.c_str());
    mlog(LOG_INFO, "oid       : %s", this->oid.c_str());

    return result;
}

const QVersionNumber *NudgisData::GetServerVersion()
{
    QVersionNumber **result = &this->server_version;
    if (*result == NULL) {
        QVersionNumber version_number = QVersionNumber::fromString(DEF_VERSION_NUMBER);
        bool getdata_result;
        std::string getdata_response = this->GetData(this->GetApiBaseUrl(), this->GetApiBaseGetdata(), &getdata_result);
        if (getdata_result) {
            obs_data_t *obs_data = obs_data_create_from_json(getdata_response.c_str());
            if (obs_data != NULL) {
                const char *mediaserver = obs_data_get_string(obs_data, "mediaserver");
                if (mediaserver != NULL && strlen(mediaserver) > 0)
                    version_number = QVersionNumber::fromString(mediaserver);
                obs_data_release(obs_data);
            }
        }
        *result = new QVersionNumber(version_number);
    }
    return *result;
}

const std::string &NudgisData::GetChannel(std::string &config_channel)
{
    static std::string result;

    if (config_channel == DEF_CHANNEL) {
        if (this->oid_personal_channel == OID_PERSONAL_CHANNEL_UNDEF) {
            bool getdata_result;
            std::string getdata_response = this->GetData(this->GetChannelsPersonalUrl(), this->GetChannelsPersonalGetdata(), &getdata_result);
            if (getdata_result) {
                obs_data_t *obs_data = obs_data_create_from_json(getdata_response.c_str());
                if (obs_data != NULL) {
                    this->oid_personal_channel = obs_data_get_string(obs_data, "oid");
                    obs_data_release(obs_data);
                }
            }
        }
        result = this->oid_personal_channel;
    } else
        result = config_channel;

    return result;
}

const std::string &NudgisData::GetUploadChannel()
{
    return this->GetChannel(this->nudgis_config->upload_channel);
}

const std::string &NudgisData::GetStreamChannel()
{
    return this->GetChannel(this->nudgis_config->stream_channel);
}

const std::string &NudgisData::GetPrepareUrl()
{
    static std::string result;

    std::ostringstream prepare_url;
    prepare_url << this->nudgis_config->url << PATH_PREPARE_URL;
    result = prepare_url.str();
    mlog(LOG_DEBUG, "prepare_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetPreparePostdata(const NudgisStreams *nudgis_streams)
{
    static std::string result;

    std::ostringstream prepare_postdata;
    prepare_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_MULTI_STREAMS << DEF_MULTI_STREAMS << "&" << PARAM_STREAMS << nudgis_streams->GetJson() << "&" << PARAM_TITLE << this->nudgis_config->stream_title << "&" << PARAM_CHANNEL << this->GetStreamChannel();
    result = prepare_postdata.str();
    mlog(LOG_DEBUG, "prepare_postdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetStartUrl()
{
    static std::string result;

    std::ostringstream start_url;
    start_url << this->nudgis_config->url << PATH_START_URL;
    result = start_url.str();
    mlog(LOG_DEBUG, "start_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetStartPostdata()
{
    static std::string result;

    std::ostringstream start_postdata;
    start_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
    result = start_postdata.str();
    mlog(LOG_DEBUG, "start_postdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetStopUrl()
{
    static std::string result;

    std::ostringstream stop_url;
    stop_url << this->nudgis_config->url << PATH_STOP_URL;
    result = stop_url.str();
    mlog(LOG_DEBUG, "stop_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetStopPostdata()
{
    static std::string result;

    std::ostringstream stop_postdata;
    stop_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
    result = stop_postdata.str();
    mlog(LOG_DEBUG, "stop_postdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetApiBaseUrl()
{
    static std::string result;

    std::ostringstream apibase_url;
    apibase_url << this->nudgis_config->url << PATH_API_BASE_URL;
    result = apibase_url.str();
    mlog(LOG_DEBUG, "api_base_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetApiBaseGetdata()
{
    static std::string result;

    std::ostringstream apibaseurl_getdata;
    apibaseurl_getdata << PARAM_API_KEY << this->nudgis_config->api_key;
    result = apibaseurl_getdata.str();
    mlog(LOG_DEBUG, "apibaseurl_getdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetUploadUrl()
{
    static std::string result;

    std::ostringstream upload_url;
    upload_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_UPLOAD_URL;
    result = upload_url.str();
    mlog(LOG_DEBUG, "upload_url: %s", result.c_str());

    return result;
}

std::list<HttpClientFormField> &NudgisData::GetUploadFormFields(std::string &file_basename, const char *read_buffer, size_t chunk, std::string &upload_id)
{
    static std::list<HttpClientFormField> result;

    result = {
            {"api_key", "", this->nudgis_config->api_key.c_str(), 0},
            {"file", file_basename, read_buffer, chunk},
    };

    if (upload_id.length() > 0)
        result.push_front({"upload_id", "", upload_id.c_str(), 0});

    return result;
}

const std::string &NudgisData::GetUploadCompleteUrl()
{
    static std::string result;

    std::ostringstream uploadcomplete_url;
    uploadcomplete_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_UPLOADCOMPLETE_URL;
    result = uploadcomplete_url.str();
    mlog(LOG_DEBUG, "uploadcomplete_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetUploadCompletePostdata(std::string &upload_id, bool check_md5, QCryptographicHash &md5sum)
{
    static std::string result;

    std::ostringstream uploadcomplete_postdata;
    uploadcomplete_postdata << PARAM_UPLOAD_ID << upload_id << "&" << PARAM_API_KEY << this->nudgis_config->api_key << "&";
    if (check_md5)
        uploadcomplete_postdata << PARAM_MD5 << md5sum.result().toHex().toStdString();
    else
        uploadcomplete_postdata << PARAM_NO_MD5 << "yes";
    result = uploadcomplete_postdata.str();
    mlog(LOG_DEBUG, "uploadcomplete_postdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetMediasAddUrl()
{
    static std::string result;

    std::ostringstream mediasadd_url;
    mediasadd_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_MEDIASADD_URL;
    result = mediasadd_url.str();
    mlog(LOG_DEBUG, "mediasadd_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetMediasAddPostdata(std::string &upload_id, std::string &title)
{
    static std::string result;

    std::ostringstream mediasadd_postdata;
    mediasadd_postdata << PARAM_ORIGIN << ORIGIN << "&" << PARAM_CODE << upload_id << "&" << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_TITLE << title << "&" << PARAM_CHANNEL << this->GetUploadChannel();
    result = mediasadd_postdata.str();
    mlog(LOG_DEBUG, "mediasadd_postdata: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetChannelsPersonalUrl()
{
    static std::string result;

    std::ostringstream channelspersonal_url;
    channelspersonal_url << this->nudgis_config->url << PATH_CHANNELS_PERSONAL_URL;
    result = channelspersonal_url.str();
    mlog(LOG_DEBUG, "channelspersonal_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetChannelsPersonalGetdata()
{
    static std::string result;

    std::ostringstream channelspersonal_getdata;
    channelspersonal_getdata << PARAM_API_KEY << this->nudgis_config->api_key;
    result = channelspersonal_getdata.str();
    mlog(LOG_DEBUG, "channelspersonal_getdata: %s", result.c_str());

    return result;
}

uint64_t NudgisData::GetUploadChunkSize()
{
    return this->nudgis_config->upload_chunk_size;
}

const std::string &NudgisData::GetMediasDeleteUrl()
{
    static std::string result;

    std::ostringstream medias_delete_url;
    medias_delete_url << this->nudgis_config->url << PATH_MEDIAS_DELETE_URL;
    result = medias_delete_url.str();
    mlog(LOG_DEBUG, "medias_delete_url: %s", result.c_str());

    return result;
}

const std::string &NudgisData::GetMediasDeletePostdata(bool delete_metadata, bool delete_resources)
{
    static std::string result;

    std::ostringstream medias_delete_postdata;
    medias_delete_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid << "&" << PARAM_DELETE_METADATA << delete_metadata << "&" << PARAM_DELETE_RESOURCES << delete_resources;
    result = medias_delete_postdata.str();
    mlog(LOG_DEBUG, "medias_delete_postdata: %s", result.c_str());

    return result;
}

enum NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT NudgisData::TestLive()
{
    enum NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT result = NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT_FAILED;

    std::string backup_stream_title = this->nudgis_config->stream_title;
    std::string backup_stream_channel = this->nudgis_config->stream_channel;

    std::ostringstream stream_title;
    stream_title << PREFIX_TEST_STREAM_TITLE << QRandomGenerator::global()->bounded(10000, 99999);
    this->nudgis_config->stream_title = stream_title.str();
    this->nudgis_config->stream_channel = DEF_CHANNEL;

    NudgisStreams nudgis_streams{};

    this->PostData(this->GetPrepareUrl(), this->GetPreparePostdata(&nudgis_streams));
    if (this->http_client.getSendSuccess()) {
        this->InitFromPrepareResponse(this->http_client.getResponse());
        this->PostData(this->GetMediasDeleteUrl(), this->GetMediasDeletePostdata(true, true));
    }

    if (this->http_client.getSendSuccess())
        result = NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT_SUCESS;

    this->nudgis_config->stream_title = backup_stream_title;
    this->nudgis_config->stream_channel = backup_stream_channel;

    mlog(LOG_DEBUG, "TestLive result: %s", NUDGISDATA_LIVE_TEST_RESULT_STR[result]);

    return result;
}

static void update_video_keyint_sec(int new_value, obs_output_t *output)
{
    obs_data_t *settings;
    obs_encoder_t *venc;
    int old_value;
    const char *path = obs_frontend_get_current_profile_path(FILENAME_STREAMENCODER);

    settings = obs_data_create_from_json_file_safe(path, "bak");
    old_value = obs_data_get_int(settings, "keyint_sec");

    if (old_value != new_value) {
        obs_data_set_int(settings, "keyint_sec", new_value);
        obs_data_save_json_safe(settings, path, "tmp", "bak");
    }
    obs_data_release(settings);

    venc = obs_output_get_video_encoder(output);
    settings = obs_encoder_get_settings(venc);
    old_value = obs_data_get_int(settings, "keyint_sec");
    if (old_value != new_value)
        obs_data_set_int(settings, "keyint_sec", new_value);
    obs_data_release(settings);
}

static const char *nudgis_name(void *unused)
{
    UNUSED_PARAMETER(unused);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    return obs_module_text(NUDGIS_NAME);
}

static void nudgis_destroy(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = (NudgisData *)data;
    delete nudgis_data;
}

static void *nudgis_create(obs_data_t *settings, obs_service_t *service)
{
    UNUSED_PARAMETER(settings);
    UNUSED_PARAMETER(service);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = new NudgisData(settings);
    return nudgis_data;
}

static bool nudgis_initialize(void *data, obs_output_t *output)
{
    bool result = false;
    UNUSED_PARAMETER(output);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = (NudgisData *)data;

    update_video_keyint_sec(DEF_KEYINT_SEC, output);

    NudgisStreams nudgis_streams{output};

    std::string prepare_response = nudgis_data->PostData(nudgis_data->GetPrepareUrl(), nudgis_data->GetPreparePostdata(&nudgis_streams), &result);
    if (result) {
        nudgis_data->InitFromPrepareResponse(prepare_response);
        result = nudgis_data->PostData(nudgis_data->GetStartUrl(), nudgis_data->GetStartPostdata());
    }

    return result;
}

static const char *nudgis_url(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = (NudgisData *)data;
    return nudgis_data->server_uri.c_str();
}

static const char *nudgis_key(void *data)
{
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = (NudgisData *)data;
    return nudgis_data->stream_id.c_str();
}

static void nudgis_deactivate(void *data)
{
    UNUSED_PARAMETER(data);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisData *nudgis_data = (NudgisData *)data;
    nudgis_data->PostData(nudgis_data->GetStopUrl(), nudgis_data->GetStopPostdata());
}

static obs_properties_t *nudgis_properties(void *data)
{
    UNUSED_PARAMETER(data);
    mlog(LOG_DEBUG, "Enter in %s", __func__);
    NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();
    obs_property_t *p;
    obs_properties_t *ppts = obs_properties_create();
    p = obs_properties_add_list(ppts, "service", obs_module_text("Service"),
                                OBS_COMBO_TYPE_LIST,
                                OBS_COMBO_FORMAT_STRING);
    obs_property_list_add_string(p, nudgis_name(data), NUDGIS_NAME);

    p = obs_properties_add_list(ppts, "server", obs_module_text("Server"),
                                OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);

    obs_property_list_add_string(p, obs_module_text(nudgis_config->url.c_str()), nudgis_config->url.c_str());

    return ppts;
}

struct obs_service_info nudgis_service_info =
        {
                /* required */
                "nudgis", // const char *id;

                nudgis_name,    //const char *(*get_name)(void *type_data);
                nudgis_create,  //void *(*create)(obs_data_t *settings, obs_service_t *service);
                nudgis_destroy, //void (*destroy)(void *data);

                /* optional */
                NULL,              //void (*activate)(void *data, obs_data_t *settings);
                nudgis_deactivate, //void (*deactivate)(void *data);

                NULL, //void (*update)(void *data, obs_data_t *settings);

                NULL, //void (*get_defaults)(obs_data_t *settings);

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

                nudgis_url, //const char *(*get_url)(void *data);
                nudgis_key, // const char *(*get_key)(void *data);

                NULL, //const char *(*get_username)(void *data);
                NULL, //const char *(*get_password)(void *data);

                NULL, //bool (*deprecated_1)();

                NULL, //void (*apply_encoder_settings)(void *data,
                      //       obs_data_t *video_encoder_settings,
                      //       obs_data_t *audio_encoder_settings);

                NULL, //void *type_data;
                NULL, //void (*free_type_data)(void *type_data);

                NULL, //const char *(*get_output_type)(void *data);

                NULL, //void (*get_supported_resolutions)(
                      //void *data, struct obs_service_resolution **resolutions,
                      //size_t *count);
                NULL, //void (*get_max_fps)(void *data, int *fps);

                NULL, //void (*get_max_bitrate)(void *data, int *video_bitrate,
                      //int *audio_bitrate);
};

NudgisUpload::NudgisUpload(const char *filename)
{
    if (filename != NULL)
        this->filename = strdup(filename);
}

NudgisUpload::~NudgisUpload()
{
    if (this->filename != NULL)
        delete this->filename;
}

void NudgisUpload::run()
{
    this->state = NUDGIS_UPLOAD_STATE_UPLOAD_IN_PROGRESS;
    std::string file_basename = QFileInfo(this->filename).fileName().toStdString();
    mlog(LOG_INFO, "enter in nudgis_upload_file with filename: %s (%s)", this->filename, file_basename.c_str());
    QCryptographicHash md5sum(QCryptographicHash::Md5);
    std::ifstream file(this->filename, std::ifstream::ate | std::ifstream::binary);
    if (file.is_open()) {
        std::streampos total_size = file.tellg();
        uint64_t chunk_size = this->nudgis_data.GetUploadChunkSize();
        uint64_t chunks_count = ceil(total_size * 1.0 / chunk_size);
        uint64_t chunk_index = 0;
        uint64_t previous_offset = 0;
        uint64_t current_offset = 0;

        HttpClient *http_client = &this->nudgis_data.GetHttpClient();
        this->http_client_error = &http_client->getError();
        http_client->reset();

        file.seekg(current_offset, std::ios::beg);
        char *read_buffer = new char[chunk_size];
        if (read_buffer != NULL) {
            std::string upload_id;
            std::string upload_url = this->nudgis_data.GetUploadUrl();

            if (http_client->getSendSuccess()) {
                http_client->setUrl(upload_url.c_str());
                http_client->setMethod(HttpClient::HTTP_CLIENT_METHOD_POST);

                while (!file.eof() && !this->canceled && http_client->getSendSuccess()) {
                    file.read(read_buffer, chunk_size);
                    std::streamsize chunk = file.gcount();
                    current_offset += chunk;
                    chunk_index++;
                    mlog(LOG_INFO, "Uploading chunk %lu/%lu.", chunk_index, chunks_count);
                    if (this->check_md5)
                        md5sum.addData(read_buffer, chunk);

                    std::ostringstream headers;
                    headers << "Content-Range: bytes " << previous_offset << "-" << current_offset - 1 << "/" << total_size;

                    std::vector<std::string> extraHeaders =
                            {
                                    headers.str(),
                                    "Expect:",
                                    "Accept-Language: en",
                            };

                    http_client->setHeaders(extraHeaders);
                    http_client->setFormFields(this->nudgis_data.GetUploadFormFields(file_basename, read_buffer, chunk, upload_id));

                    http_client->send();
                    if (http_client->getSendSuccess()) {
                        mlog(LOG_INFO, "90.0 * current_offset / total_size: %f", 90.0 * current_offset / total_size);
                        emit this->progressUpload(90.0 * current_offset / total_size);
                        if (upload_id.length() < 1 && obs_data_has_user_value(http_client->getResponseObsData(), "upload_id"))
                            upload_id = obs_data_get_string(http_client->getResponseObsData(), "upload_id");
                        previous_offset = current_offset;
                    }
                }

                if (!this->canceled && http_client->getSendSuccess()) {
                    http_client->reset();
                    if (this->nudgis_data.PostData(this->nudgis_data.GetUploadCompleteUrl(), this->nudgis_data.GetUploadCompletePostdata(upload_id, this->check_md5, md5sum)) && !this->canceled) {
                        if (this->nudgis_data.PostData(this->nudgis_data.GetMediasAddUrl(), this->nudgis_data.GetMediasAddPostdata(upload_id, file_basename))) {
                            if (obs_data_has_user_value(this->nudgis_data.GetHttpClient().getResponseObsData(), "oid"))
                                this->file_uploaded_oid = obs_data_get_string(this->nudgis_data.GetHttpClient().getResponseObsData(), "oid");
                            this->state = NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL;
                        }
                    }

                    emit this->progressUpload(100);
                }
            }

            if (!http_client->getSendSuccess())
                this->state = NUDGIS_UPLOAD_STATE_UPLOAD_FAILED;
            delete[] read_buffer;
        }

        file.close();
    }

    if (this->canceled)
        this->state = NUDGIS_UPLOAD_STATE_UPLOAD_CANCEL;

    mlog(LOG_DEBUG, "  NudgisUpload state: %s", NUDGIS_UPLOAD_STATE_STR[this->state]);

    emit this->endUpload();
}

void NudgisUpload::cancel()
{
    this->canceled = true;
}

void NudgisUpload::setCheckMd5(bool enabled)
{
    this->check_md5 = enabled;
}

const char *NudgisUpload::GetFileUploadedUrlHtml()
{
    static std::string result;
    std::ostringstream file_uploaded_url_html;

    std::string file_uploaded_url = this->GetFileUploadedUrl();
    file_uploaded_url_html << "<a href=\"" << file_uploaded_url << "\">" << file_uploaded_url << "</a>";
    result = file_uploaded_url_html.str();

    return result.c_str();
}

std::string &NudgisUpload::GetFileUploadedUrl()
{
    static std::string result;
    std::ostringstream file_uploaded_url;

    file_uploaded_url << this->nudgis_data.nudgis_config->url << "/permalink/" << this->GetFileUploadedOid() << "/";
    result = file_uploaded_url.str();

    return result;
}

std::string &NudgisUpload::GetFileUploadedOid()
{
    return this->file_uploaded_oid;
}

NudgisUpload::NUDGIS_UPLOAD_STATE NudgisUpload::GetState()
{
    return this->state;
}

const HttpClientError *NudgisUpload::GetHttpClientError()
{
    return this->http_client_error;
}
