#ifndef NUDGIS_SERVICE_HPP
#define NUDGIS_SERVICE_HPP

#include "nudgis-config.hpp"
#include "obs-utils.hpp"
#include "http-client.hpp"

#include <QtCore>
#include <cstddef>
#include <obs.h>

#define DEF_SERVER_URI "rtmp"
#define DEF_STREAM_ID "stream_id"
#define DEF_OID "oid"

#define OID_PERSONAL_CHANNEL_UNDEF "undef"

extern struct obs_service_info nudgis_service_info;

class NudgisTestParamsData {
public:
	enum NUDGISDATA_LIVE_TEST_RESULT {
		NUDGISDATA_LIVE_TEST_RESULT_SUCESS,
		NUDGISDATA_LIVE_TEST_RESULT_FAILED,
	};

	enum NUDGISDATA_LIVE_TEST_RESULT result;
	const HttpClientError *http_client_error;
};

class NudgisStreams {
public:
	NudgisStreams();
	NudgisStreams(int width, int height, int video_bitrate, int audio_bitrate, int framerate);
	NudgisStreams(obs_output_t *output);

	int width;
	int height;
	int video_bitrate;
	int audio_bitrate;
	int framerate;

	const std::string GetJson() const;
};

class NudgisData {
public:
	std::string server_uri = DEF_SERVER_URI;
	std::string stream_id = DEF_STREAM_ID;
	std::string oid = DEF_OID;
	std::string oid_personal_channel = OID_PERSONAL_CHANNEL_UNDEF;
	NudgisConfig *nudgis_config = NULL;

	NudgisData(NudgisConfig *nudgis_config = NULL);
	NudgisData(obs_data_t *settings, NudgisConfig *nudgis_config = NULL);
	~NudgisData();

	HttpClient &GetHttpClient();
	const std::string &GetUrlPrefix();
	const std::string &GetData(const std::string &url, const std::string &getData, bool *result);
	const std::string &PostData(const std::string &url, const std::string &postData, bool *result);
	bool PostData(const std::string &url, const std::string &postData);
	bool InitFromPrepareResponse(const std::string &prepare_response);
	const QVersionNumber *GetServerVersion();
	const std::string &GetChannel(std::string &config_channel);
	const std::string &GetUploadChannel();
	const std::string &GetStreamChannel();
	const std::string &GetPrepareUrl();
	const std::string &GetPreparePostdata(const NudgisStreams *nudgis_streams);
	const std::string &GetStartUrl();
	const std::string &GetStartPostdata();
	const std::string &GetStopUrl();
	const std::string &GetStopPostdata();
	const std::string &GetApiBaseUrl();
	const std::string &GetApiBaseGetdata();
	const std::string &GetUploadUrl();
	std::list<HttpClientFormField> &GetUploadFormFields(std::string &file_basename, const char *read_buffer,
							    size_t chunk, std::string &upload_id);
	const std::string &GetUploadCompleteUrl();
	const std::string &GetUploadCompletePostdata(std::string &upload_id, bool check_md5,
						     QCryptographicHash &md5sum);
	const std::string &GetMediasAddUrl();
	const std::string &GetMediasAddPostdata(std::string &upload_id, std::string &title);
	const std::string &GetChannelsPersonalUrl();
	const std::string &GetChannelsPersonalGetdata();
	uint64_t GetUploadChunkSize();
	const std::string &GetMediasDeleteUrl();
	const std::string &GetMediasDeletePostdata(bool delete_metadata = false, bool delete_resources = false);

	enum NudgisTestParamsData::NUDGISDATA_LIVE_TEST_RESULT TestLive();

private:
	obs_data_t *settings = NULL;
	QVersionNumber *server_version = NULL;
	std::string *url_prefix = NULL;
	HttpClient http_client;

	bool GetResponseSuccess(obs_data_t *obs_data);
	bool GetResponseSuccess(const std::string &response);
};

class NudgisUpload : public QObject {
	Q_OBJECT;

public:
	enum NUDGIS_UPLOAD_STATE {
		NUDGIS_UPLOAD_STATE_IDLE,
		NUDGIS_UPLOAD_STATE_UPLOAD_IN_PROGRESS,
		NUDGIS_UPLOAD_STATE_UPLOAD_SUCESSFULL,
		NUDGIS_UPLOAD_STATE_UPLOAD_CANCEL,
		NUDGIS_UPLOAD_STATE_UPLOAD_FAILED,
	};
	NudgisUpload(const char *filename);
	~NudgisUpload();
	void run();
	void cancel();
	void setCheckMd5(bool enabled);
	const char *GetFileUploadedUrlHtml();
	std::string &GetFileUploadedUrl();
	std::string &GetFileUploadedOid();
	NUDGIS_UPLOAD_STATE GetState();
	const HttpClientError *GetHttpClientError();

private:
	NUDGIS_UPLOAD_STATE state = NUDGIS_UPLOAD_STATE_IDLE;
	bool check_md5 = true;
	bool canceled = false;
	const char *filename = NULL;
	NudgisData nudgis_data;
	const HttpClientError *http_client_error = NULL;
	std::string file_uploaded_oid = {};

signals:
	void progressUpload(int percent);
	void endUpload();
};

#endif
