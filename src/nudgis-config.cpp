#include "nudgis-config.hpp"
#include "plugin-macros.generated.h"
#include "obs-utils.hpp"

#include <obs.hpp>

#define DEF_URL "https://my.ubicast.tv"
#define DEF_API_KEY ""
#define DEF_STREAM_TITLE "Title"
#define DEF_STREAM_CHANNEL "Personal channel"
#define DEF_AUTOSTATES AutoStates[ASK]
#define DEF_AUTO_DELETE_UPLOADED_FILE DEF_AUTOSTATES
#define DEF_PUBLISH_RECORDING_AUTOMATICALLY DEF_AUTOSTATES

static NudgisConfig currentNudgisConfig;

enum AutoStates{
    ASK,
    NEVER,
    YES,
};

static const char * AutoStates[]
{
    [ASK] = "NudgisPlugin.settings.Ask",
    [NEVER] = "NudgisPlugin.settings.Never",
    [YES] = "NudgisPlugin.settings.Yes",
};

#define AUTOSTATES_LEN (sizeof(AutoStates)/sizeof(AutoStates[0]))

NudgisConfig::NudgisConfig()
{
    this->url = DEF_URL;
    this->api_key = DEF_API_KEY;
    this->stream_title = DEF_STREAM_TITLE;
    this->stream_channel = DEF_STREAM_CHANNEL;
    this->auto_delete_uploaded_file = DEF_AUTO_DELETE_UPLOADED_FILE;
    this->publish_recording_automatically = DEF_PUBLISH_RECORDING_AUTOMATICALLY;
}


const char** NudgisConfig::GetAllAutoStates()
{
    return AutoStates;
}

size_t NudgisConfig::GetAutoStatesCount()
{
    return AUTOSTATES_LEN;
}

const char * NudgisConfig::FindAutoState(const char *str)
{
    const char * result = NULL;
    for (size_t i=0;str != NULL && i < AUTOSTATES_LEN && result == NULL;i++)
    {
        if (!strcmp(str,AutoStates[i]))
            result = AutoStates[i];
    }
    return result ? result : DEF_AUTOSTATES;
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

        obs_data_set_default_string(data, "auto_delete_uploaded_file", DEF_AUTO_DELETE_UPLOADED_FILE);
        this->auto_delete_uploaded_file = this->FindAutoState(obs_data_get_string(data, "auto_delete_uploaded_file"));

        obs_data_set_default_string(data, "publish_recording_automatically", DEF_PUBLISH_RECORDING_AUTOMATICALLY);
        this->publish_recording_automatically = this->FindAutoState(obs_data_get_string(data, "publish_recording_automatically"));

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
    obs_data_set_string(data, "auto_delete_uploaded_file", this->auto_delete_uploaded_file);
    obs_data_set_string(data, "publish_recording_automatically", this->publish_recording_automatically);

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