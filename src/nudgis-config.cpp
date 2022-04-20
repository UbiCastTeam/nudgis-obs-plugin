#include "nudgis-config.hpp"
#include "plugin-macros.generated.h"
#include "obs-utils.hpp"

#include <obs.hpp>
#include <vector>
#include <string>
#include <algorithm>

#define DEF_URL "https://my.ubicast.tv"
#define DEF_API_KEY ""
#define DEF_STREAM_TITLE "Title"
#define DEF_STREAM_CHANNEL "Personal channel"
#define DEF_AUTOSTATES (&auto_states[0])
#define DEF_AUTO_DELETE_UPLOADED_FILE DEF_AUTOSTATES
#define DEF_PUBLISH_RECORDING_AUTOMATICALLY DEF_AUTOSTATES
#define DEF_UPLOAD_CHUNK_SIZE 5242880

static const vector<AutoState> auto_states{
        {"NudgisPlugin.settings.Ask", AutoState::AUTOSTATE_ASK},
        {"NudgisPlugin.settings.Never", AutoState::AUTOSTATE_NEVER},
        {"NudgisPlugin.settings.Yes", AutoState::AUTOSTATE_YES},
};

static NudgisConfig currentNudgisConfig;

const vector<AutoState> &AutoState::GetAll()
{
    return auto_states;
}

const AutoState &AutoState::Find(const char *str)
{
    const AutoState *result = DEF_AUTOSTATES;
    if (count_if(auto_states.begin(), auto_states.end(), [str](const AutoState &autostate) { return (autostate.name == str); }) > 0)
        result = &*find_if(auto_states.begin(), auto_states.end(), [str](const AutoState &autostate) { return (autostate.name == str); });
    return *result;
}

NudgisConfig::NudgisConfig()
{
    this->url = DEF_URL;
    this->api_key = DEF_API_KEY;
    this->stream_title = DEF_STREAM_TITLE;
    this->stream_channel = DEF_STREAM_CHANNEL;
    this->auto_delete_uploaded_file = DEF_AUTO_DELETE_UPLOADED_FILE;
    this->publish_recording_automatically = DEF_PUBLISH_RECORDING_AUTOMATICALLY;
    this->upload_chunk_size = DEF_UPLOAD_CHUNK_SIZE;
}

void NudgisConfig::load(const char *filename)
{
    const char *path = obs_frontend_get_current_profile_path(filename);

    obs_data_t *data = obs_data_create_from_json_file_safe(path, "bak");

    if (data != NULL) {
        obs_data_set_default_string(data, "url", DEF_URL);
        this->url = obs_data_get_string(data, "url");

        obs_data_set_default_string(data, "api_key", DEF_API_KEY);
        this->api_key = obs_data_get_string(data, "api_key");

        obs_data_set_default_string(data, "stream_title", DEF_STREAM_TITLE);
        this->stream_title = obs_data_get_string(data, "stream_title");

        obs_data_set_default_string(data, "stream_channel", DEF_STREAM_CHANNEL);
        this->stream_channel = obs_data_get_string(data, "stream_channel");

        obs_data_set_default_string(data, "auto_delete_uploaded_file", DEF_AUTO_DELETE_UPLOADED_FILE->name.c_str());
        this->auto_delete_uploaded_file = &AutoState::Find(obs_data_get_string(data, "auto_delete_uploaded_file"));

        obs_data_set_default_string(data, "publish_recording_automatically", DEF_PUBLISH_RECORDING_AUTOMATICALLY->name.c_str());
        this->publish_recording_automatically = &AutoState::Find(obs_data_get_string(data, "publish_recording_automatically"));

        obs_data_set_default_int(data, "upload_chunk_size", DEF_UPLOAD_CHUNK_SIZE);
        this->upload_chunk_size = obs_data_get_int(data, "upload_chunk_size");

        obs_data_release(data);
    }
}

void NudgisConfig::save(const char *filename)
{
    const char *path = obs_frontend_get_current_profile_path(filename);

    obs_data_t *data = obs_data_create();

    obs_data_set_string(data, "url", this->url.c_str());
    obs_data_set_string(data, "api_key", this->api_key.c_str());
    obs_data_set_string(data, "stream_title", this->stream_title.c_str());
    obs_data_set_string(data, "stream_channel", this->stream_channel.c_str());
    obs_data_set_string(data, "auto_delete_uploaded_file", this->auto_delete_uploaded_file->name.c_str());
    obs_data_set_string(data, "publish_recording_automatically", this->publish_recording_automatically->name.c_str());
    obs_data_set_int(data, "upload_chunk_size", this->upload_chunk_size);

    if (!obs_data_save_json_safe(data, path, "tmp", "bak"))
        mlog(LOG_WARNING, "%s", "Failed to save nudgis_config");

    obs_data_release(data);
}

NudgisConfig *NudgisConfig::GetCurrentNudgisConfig()
{
    static bool first_call = true;
    if (first_call) {
        currentNudgisConfig.load();
        first_call = false;
    }
    return &currentNudgisConfig;
}