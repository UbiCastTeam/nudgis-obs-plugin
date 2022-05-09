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

#define FILENAME_STREAMENCODER "streamEncoder.json"

static const string &GetRemoteFile(const string &url, const string &postData, bool *result = nullptr)
{
    static string response;
    string error = {};
    response = "";
    bool get_remote_file = GetRemoteFile(url.c_str(), response, error, nullptr, nullptr, "", postData.length() > 0 ? postData.c_str() : nullptr);
    if (result != NULL)
        *result = get_remote_file;
    mlog(LOG_DEBUG, "GetRemoteFile (%s) response: %s", get_remote_file ? "OK" : "NOK", response.c_str());
    return response;
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

    bool NudgisData::GetResponseSuccess(const string &response)
    {
        bool result = false;
        obs_data_t *obs_data = obs_data_create_from_json(response.c_str());
        if (obs_data != NULL) {
            result = this->GetResponseSuccess(obs_data);
            obs_data_release(obs_data);
        }
        return result;
    }

    const string &NudgisData::GetJsonStreams(obs_output_t *output)
    {
        static string result;

        int width;
        int height;
        int video_bitrate;
        int audio_bitrate;
        int framerate;

        obs_encoder_t *venc = obs_output_get_video_encoder(output);
        obs_encoder_t *aenc = obs_output_get_audio_encoder(output, 0);
        obs_data_t *vsettings = obs_encoder_get_settings(venc);
        obs_data_t *asettings = obs_encoder_get_settings(aenc);

        video_bitrate = obs_data_get_int(vsettings, "bitrate") * 1000;
        audio_bitrate = obs_data_get_int(asettings, "bitrate") * 1000;

        obs_data_release(vsettings);
        obs_data_release(asettings);

        const struct video_output_info *output_video_info = video_output_get_info(obs_output_video(output));
        width = output_video_info->width;
        height = output_video_info->height;
        framerate = output_video_info->fps_num;

        json_t *streams = json_array();
        if (streams != NULL) {
            json_t *stream = json_object();
            if (stream != NULL) {
                json_object_set(stream, "width", json_integer(width));
                json_object_set(stream, "height", json_integer(height));
                json_object_set(stream, "video_bitrate", json_integer(video_bitrate));
                json_object_set(stream, "audio_bitrate", json_integer(audio_bitrate));
                json_object_set(stream, "framerate", json_integer(framerate));

                json_array_append_new(streams, stream);
            }

            result = json_dumps(streams, 0);
            json_decref(streams);
        }

        return result;
    }

    NudgisData::NudgisData()
    {
        this->settings = NULL;
    }

    NudgisData::NudgisData(obs_data_t *settings)
    {
        this->settings = settings;
    }

    NudgisData::~NudgisData()
    {
        if (server_version != NULL)
            delete server_version;
        if (url_prefix != NULL)
            delete url_prefix;
    }

    const string &NudgisData::GetUrlPrefix()
    {
        if (this->url_prefix == NULL) {
            this->url_prefix = new string(*this->GetServerVersion() < QVersionNumber(8, 2) ? "medias/resource/" : "");
        }
        return *this->url_prefix;
    }

    const string &NudgisData::GetData(const string &url, const string &getData, bool *result)
    {
        return this->PostData(url + "?" + getData, "", result);
    }

    const string &NudgisData::PostData(const string &url, const string &postData, bool *result)
    {
        bool get_remote_file;
        const string &response = GetRemoteFile(url, postData, &get_remote_file);
        if (result != NULL) {
            *result = false;
            if (get_remote_file)
                *result = this->GetResponseSuccess(response);
        }
        return response;
    }

    bool NudgisData::PostData(const string &url, const string &postData)
    {
        bool result;
        this->PostData(url, postData, &result);
        return result;
    }

    bool NudgisData::InitFromPrepareResponse(const string &prepare_response)
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
            string getdata_response = this->GetData(this->GetApiBaseUrl(), this->GetApiBaseGetdata(), &getdata_result);
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

    const string &NudgisData::GetStreamChannel()
    {
        static string result;

        if (this->nudgis_config->stream_channel == DEF_STREAM_CHANNEL)
        {
            if (this->oid_personal_channel == OID_PERSONAL_CHANNEL_UNDEF)
            {
                bool getdata_result;
                string getdata_response = this->GetData(this->GetChannelsPersonalUrl(), this->GetChannelsPersonalGetdata(), &getdata_result);
                if (getdata_result) {
                    obs_data_t *obs_data = obs_data_create_from_json(getdata_response.c_str());
                    if (obs_data != NULL) {
                        this->oid_personal_channel = obs_data_get_string(obs_data, "oid");
                        obs_data_release(obs_data);
                    }
                }
            }
            result = this->oid_personal_channel;
        }
        else
            result = this->nudgis_config->stream_channel;

        return result;
    }

    const string &NudgisData::GetPrepareUrl()
    {
        static string result;

        ostringstream prepare_url;
        prepare_url << this->nudgis_config->url << PATH_PREPARE_URL;
        result = prepare_url.str();
        mlog(LOG_DEBUG, "prepare_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetPreparePostdata(obs_output_t *output)
    {
        static string result;

        ostringstream prepare_postdata;
        prepare_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_MULTI_STREAMS << DEF_MULTI_STREAMS << "&" << PARAM_STREAMS << GetJsonStreams(output) << "&" << PARAM_TITLE << this->nudgis_config->stream_title << "&" << PARAM_CHANNEL << this->GetStreamChannel();
        result = prepare_postdata.str();
        mlog(LOG_DEBUG, "prepare_postdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetStartUrl()
    {
        static string result;

        ostringstream start_url;
        start_url << this->nudgis_config->url << PATH_START_URL;
        result = start_url.str();
        mlog(LOG_DEBUG, "start_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetStartPostdata()
    {
        static string result;

        ostringstream start_postdata;
        start_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
        result = start_postdata.str();
        mlog(LOG_DEBUG, "start_postdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetStopUrl()
    {
        static string result;

        ostringstream stop_url;
        stop_url << this->nudgis_config->url << PATH_STOP_URL;
        result = stop_url.str();
        mlog(LOG_DEBUG, "stop_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetStopPostdata()
    {
        static string result;

        ostringstream stop_postdata;
        stop_postdata << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_OID << this->oid;
        result = stop_postdata.str();
        mlog(LOG_DEBUG, "stop_postdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetApiBaseUrl()
    {
        static string result;

        ostringstream apibase_url;
        apibase_url << this->nudgis_config->url << PATH_API_BASE_URL;
        result = apibase_url.str();
        mlog(LOG_DEBUG, "api_base_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetApiBaseGetdata()
    {
        static string result;

        ostringstream apibaseurl_getdata;
        apibaseurl_getdata << PARAM_API_KEY << this->nudgis_config->api_key;
        result = apibaseurl_getdata.str();
        mlog(LOG_DEBUG, "apibaseurl_getdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetUploadUrl()
    {
        static string result;

        ostringstream upload_url;
        upload_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_UPLOAD_URL;
        result = upload_url.str();
        mlog(LOG_DEBUG, "upload_url: %s", result.c_str());

        return result;
    }

    list<FormField> &NudgisData::GetUploadFormFields(string &file_basename, const char *read_buffer, size_t chunk, string &upload_id)
    {
        static list<FormField> result;

        result = {
                {"api_key", "", this->nudgis_config->api_key.c_str(), 0},
                {"file", file_basename, read_buffer, chunk},
        };

        if (upload_id.length() > 0)
            result.push_front({"upload_id", "", upload_id.c_str(), 0});

        return result;
    }

    const string &NudgisData::GetUploadCompleteUrl()
    {
        static string result;

        ostringstream uploadcomplete_url;
        uploadcomplete_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_UPLOADCOMPLETE_URL;
        result = uploadcomplete_url.str();
        mlog(LOG_DEBUG, "uploadcomplete_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetUploadCompletePostdata(string &upload_id, bool check_md5, QCryptographicHash &md5sum)
    {
        static string result;

        ostringstream uploadcomplete_postdata;
        uploadcomplete_postdata << PARAM_UPLOAD_ID << upload_id << "&" << PARAM_API_KEY << this->nudgis_config->api_key << "&";
        if (check_md5)
            uploadcomplete_postdata << PARAM_MD5 << md5sum.result().toHex().toStdString();
        else
            uploadcomplete_postdata << PARAM_NO_MD5 << "yes";
        result = uploadcomplete_postdata.str();
        mlog(LOG_DEBUG, "uploadcomplete_postdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetMediasAddUrl()
    {
        static string result;

        ostringstream mediasadd_url;
        mediasadd_url << this->GetApiBaseUrl() << this->GetUrlPrefix() << PATH_MEDIASADD_URL;
        result = mediasadd_url.str();
        mlog(LOG_DEBUG, "mediasadd_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetMediasAddPostdata(string &upload_id, string &title)
    {
        static string result;

        ostringstream mediasadd_postdata;
        mediasadd_postdata << PARAM_ORIGIN << ORIGIN << "&" << PARAM_CODE << upload_id << "&" << PARAM_API_KEY << this->nudgis_config->api_key << "&" << PARAM_TITLE << title << "&" << PARAM_CHANNEL << this->GetStreamChannel();
        result = mediasadd_postdata.str();
        mlog(LOG_DEBUG, "mediasadd_postdata: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetChannelsPersonalUrl()
    {
        static string result;

        ostringstream channelspersonal_url;
        channelspersonal_url << this->nudgis_config->url << PATH_CHANNELS_PERSONAL_URL;
        result = channelspersonal_url.str();
        mlog(LOG_DEBUG, "channelspersonal_url: %s", result.c_str());

        return result;
    }

    const string &NudgisData::GetChannelsPersonalGetdata()
    {
        static string result;

        ostringstream channelspersonal_getdata;
        channelspersonal_getdata << PARAM_API_KEY << this->nudgis_config->api_key;
        result = channelspersonal_getdata.str();
        mlog(LOG_DEBUG, "channelspersonal_getdata: %s", result.c_str());

        return result;
    }

    uint64_t NudgisData::GetUploadChunkSize()
    {
        return this->nudgis_config->upload_chunk_size;
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

    string prepare_response = nudgis_data->PostData(nudgis_data->GetPrepareUrl(), nudgis_data->GetPreparePostdata(output), &result);
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

NudgisUploadFileResult::~NudgisUploadFileResult()
{
    if (this->upload_complete_response != NULL)
        obs_data_release(this->upload_complete_response);
    if (this->media_add_response != NULL)
        obs_data_release(this->media_add_response);
}

//#define DISABLE_UPLOAD 1

NudgisUploadFileResult *nudgis_upload_file(const char *filename,NudgisUploadProgressCb nudgis_upload_progress_cb, void *cb_args, bool check_md5)
{
    NudgisUploadFileResult * result = new NudgisUploadFileResult();
    string file_basename = QFileInfo(filename).fileName().toStdString();
    mlog(LOG_INFO, "enter in nudgis_upload_file with filename: %s (%s)", filename, file_basename.c_str());
    NudgisData nudgis_data;
    QCryptographicHash md5sum(QCryptographicHash::Md5);
    ifstream file(filename, ifstream::ate | ifstream::binary);
    if (file.is_open()) {
        streampos total_size = file.tellg();
        uint64_t chunk_size = nudgis_data.GetUploadChunkSize();
        uint64_t chunks_count = ceil(total_size * 1.0 / chunk_size);
        uint64_t chunk_index = 0;
        uint64_t previous_offset=0;
        uint64_t current_offset=0;

        file.seekg(current_offset, ios::beg);
        char read_buffer[chunk_size];
        string upload_id;
        string response;
        string error;

        while (!file.eof()) {
            file.read(read_buffer, chunk_size);
            streamsize chunk = file.gcount();
            current_offset += chunk;
            chunk_index++;
            mlog(LOG_INFO, "Uploading chunk %lu/%lu.", chunk_index, chunks_count);
            if (check_md5)
                md5sum.addData(read_buffer, chunk);

            ostringstream headers;
            headers << "Content-Range: bytes " << previous_offset << "-" << current_offset - 1 << "/" << total_size;

            std::vector<std::string> extraHeaders =
                    {
                            headers.str(),
                            "Expect:",
                            "Accept-Language: en",
                    };

            response.clear();
            error.clear();

#ifndef DISABLE_UPLOAD
            GetRemoteFile(
                    nudgis_data.GetUploadUrl().c_str(),
                    response,
                    error,
                    nullptr,
                    nullptr,
                    "",
                    nullptr,
                    true,
                    nudgis_data.GetUploadFormFields(file_basename, read_buffer, chunk, upload_id),
                    extraHeaders);
#endif

            mlog(LOG_INFO, "90.0 * current_offset / total_size: %f", 90.0 * current_offset / total_size);

            if (nudgis_upload_progress_cb != NULL)
                (*nudgis_upload_progress_cb)(cb_args,90.0 * current_offset / total_size);
            //~ if progress_callback:
            //~ pdata = progress_data or dict()
            //~ progress_callback(0.9 * end_offset / total_size, **pdata)

#ifndef DISABLE_UPLOAD
            if (upload_id.length() < 1) {
                obs_data_t *response_obs_data = obs_data_create_from_json(response.c_str());
                if (response_obs_data != NULL) {
                    upload_id = obs_data_get_string(response_obs_data, "upload_id");
                    obs_data_release(response_obs_data);
                }
            }
#endif

            previous_offset = current_offset;
        }

        //~ bandwidth = total_size * 8 / ((time.time() - begin) * 1000000)
        //~ logger.debug('Upload finished, average bandwidth: %.2f Mbits/s', bandwidth)

        //~ if remote_path:
        //~ data['path'] = remote_path

#ifndef DISABLE_UPLOAD
        bool upload_complete_result;
        response = nudgis_data.PostData(nudgis_data.GetUploadCompleteUrl(), nudgis_data.GetUploadCompletePostdata(upload_id, check_md5, md5sum), &upload_complete_result);
        result->upload_complete_response = obs_data_create_from_json(response.c_str());
        if (upload_complete_result)
        {
            response = nudgis_data.PostData(nudgis_data.GetMediasAddUrl(), nudgis_data.GetMediasAddPostdata(upload_id, file_basename), NULL);
            result->media_add_response = obs_data_create_from_json(response.c_str());
        }
#endif

        if (nudgis_upload_progress_cb != NULL)
            (*nudgis_upload_progress_cb)(cb_args,100);
        //~ if progress_callback:
        //~ pdata = progress_data or dict()
        //~ progress_callback(1., **pdata)
        //~ return data['upload_id']

        file.close();
    }
    return result;
}
