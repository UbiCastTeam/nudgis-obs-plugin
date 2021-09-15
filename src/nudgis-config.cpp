#include "nudgis-config.hpp"
#include "plugin-macros.generated.h"
#include "obs-utils.hpp"

#include <obs.hpp>

#define DEF_URL "https://beta.ubicast.net"
#define DEF_API_KEY ""
#define DEF_STREAM_TITLE "Title"
#define DEF_STREAM_CHANNEL "Channel"

static NudgisConfig currentNudgisConfig;

NudgisConfig::NudgisConfig()
{
    this->url = DEF_URL;
    this->api_key = DEF_API_KEY;
    this->stream_title = DEF_STREAM_TITLE;
    this->stream_channel = DEF_STREAM_CHANNEL;
}

void NudgisConfig::load(const char *filename)
{
    const char * path = obs_frontend_get_current_profile_path(filename);

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

        obs_data_release(data);
    }
}

void NudgisConfig::save(const char *filename)
{
    const char * path = obs_frontend_get_current_profile_path(filename);

    obs_data_t *data = obs_data_create();

    obs_data_set_string(data, "url", this->url.c_str());
    obs_data_set_string(data, "api_key", this->api_key.c_str());
    obs_data_set_string(data, "stream_title", this->stream_title.c_str());
    obs_data_set_string(data, "stream_channel", this->stream_channel.c_str());

    if (!obs_data_save_json_safe(data, path, "tmp", "bak"))
        mlog(LOG_WARNING, "Failed to save nudgis_config");

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