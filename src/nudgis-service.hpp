#ifndef NUDGIS_SERVICE_HPP
#define NUDGIS_SERVICE_HPP

#include "nudgis-config.hpp"
#include "obs-utils.hpp"

#include <QtCore>
#include <cstddef>
#include <obs.h>

#define DEF_SERVER_URI "rtmp"
#define DEF_STREAM_ID "stream_id"
#define DEF_OID "oid"

#define OID_PERSONAL_CHANNEL_UNDEF "undef"

using namespace std;

extern struct obs_service_info nudgis_service_info;

class NudgisData
{
    public:
        string server_uri = DEF_SERVER_URI;
        string stream_id = DEF_STREAM_ID;
        string oid = DEF_OID;
        string oid_personal_channel = OID_PERSONAL_CHANNEL_UNDEF;
        NudgisConfig *nudgis_config = NudgisConfig::GetCurrentNudgisConfig();

        NudgisData();
        NudgisData(obs_data_t *settings);
        ~NudgisData();

        const string &GetUrlPrefix();
        const string &GetData(const string &url, const string &getData, bool *result);
        const string &PostData(const string &url, const string &postData, bool *result);
        bool PostData(const string &url, const string &postData);
        bool InitFromPrepareResponse(const string &prepare_response);
        const QVersionNumber *GetServerVersion();
        const string &GetStreamChannel();
        const string &GetPrepareUrl();
        const string &GetPreparePostdata(obs_output_t *output);
        const string &GetStartUrl();
        const string &GetStartPostdata();
        const string &GetStopUrl();
        const string &GetStopPostdata();
        const string &GetApiBaseUrl();
        const string &GetApiBaseGetdata();
        const string &GetUploadUrl();
        list<FormField> &GetUploadFormFields(string &file_basename, const char *read_buffer, size_t chunk, string &upload_id);
        const string &GetUploadCompleteUrl();
        const string &GetUploadCompletePostdata(string &upload_id, bool check_md5, QCryptographicHash &md5sum);
        const string &GetMediasAddUrl();
        const string &GetMediasAddPostdata(string &upload_id, string &title);
        const string &GetChannelsPersonalUrl();
        const string &GetChannelsPersonalGetdata();
        uint64_t GetUploadChunkSize();

    private:
        obs_data_t *settings = NULL;
        QVersionNumber *server_version = NULL;
        string *url_prefix = NULL;

        bool GetResponseSuccess(obs_data_t *obs_data);
        bool GetResponseSuccess(const string &response);
        const string &GetJsonStreams(obs_output_t *output);
};

typedef void (*NudgisUploadProgressCb)(void *cb_args,int percent);

class NudgisUploadFileResult
{
    public:
        ~NudgisUploadFileResult();
        obs_data_t * upload_complete_response = NULL;
        obs_data_t * media_add_response = NULL;
};

class NudgisUpload
{

    public:
        enum NUDGIS_UPLOAD_STATE
        {
            NUDGIS_UPLOAD_STATE_IDLE,
            NUDGIS_UPLOAD_STATE_UPLOAD_IN_PROGRESS,
            NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL,
            NUDGIS_UPLOAD_STATE_UPLOAD_CANCEL,
        };
        NudgisUpload(const char *filename);
        ~NudgisUpload();
        NudgisUploadFileResult *run(NudgisUploadProgressCb nudgis_upload_progress_cb = NULL, void *cb_args = NULL);
        void cancel();
        void setCheckMd5(bool enabled);
        NUDGIS_UPLOAD_STATE GetState();

    private:
        NUDGIS_UPLOAD_STATE state = NUDGIS_UPLOAD_STATE_IDLE;
        bool check_md5 = true;
        bool canceled = false;
        const char * filename = NULL;
};

#endif
