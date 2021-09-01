#include "nudgis-config.hpp"
#include "obs.hpp"
#include <obs-utils.hpp>

#define DEF_URL            "https://beta.ubicast.net"
#define DEF_API_KEY        ""
#define DEF_STREAM_TITLE   "Test de live OBS"
#define DEF_STREAM_CHANNEL "OBS"
#define DEF_VOD_TITLE      ""
#define DEF_VOD_CHANNEL    ""

#define SIZEOF_PATH        512

static NudgisConfig currentNudgisConfig;

NudgisConfig::NudgisConfig()
{
    this->load_default();
}

void NudgisConfig::load_default()
{
    this->url = DEF_URL;
    this->api_key = DEF_API_KEY;
    this->stream_title = DEF_STREAM_TITLE;
    this->stream_channel = DEF_STREAM_CHANNEL;
    this->vod_title = DEF_VOD_TITLE;
    this->vod_channel = DEF_VOD_CHANNEL;
}

void NudgisConfig::load(const char * filename)
{
    char path[SIZEOF_PATH];
    GetProfilePath(path,SIZEOF_PATH,filename);
    
    obs_data_t *data = obs_data_create_from_json_file_safe(path, "bak");
    
    if (data != NULL)
    {
        obs_data_set_default_string(data, "url", DEF_URL);
        this->url = obs_data_get_string(data, "url");
        
        obs_data_set_default_string(data, "api_key", DEF_API_KEY);
        this->api_key = obs_data_get_string(data, "api_key");
        
        obs_data_set_default_string(data, "stream_title", DEF_STREAM_TITLE);
        this->stream_title = obs_data_get_string(data, "stream_title");
        
        obs_data_set_default_string(data, "stream_channel", DEF_STREAM_CHANNEL);
        this->stream_channel = obs_data_get_string(data, "stream_channel");
        
        obs_data_set_default_string(data, "vod_title", DEF_VOD_TITLE);
        this->vod_title = obs_data_get_string(data, "vod_title");
        
        obs_data_set_default_string(data, "vod_channel", DEF_VOD_CHANNEL);
        this->vod_channel = obs_data_get_string(data, "vod_channel");
        
        obs_data_release(data);
    }
}

void NudgisConfig::save(const char * filename)
{
    char path[SIZEOF_PATH];
    GetProfilePath(path,SIZEOF_PATH,filename);

    obs_data_t *data = obs_data_create();

    obs_data_set_string(data, "url", this->url.c_str());
    obs_data_set_string(data, "api_key", this->api_key.c_str());
    obs_data_set_string(data, "stream_title", this->stream_title.c_str());
    obs_data_set_string(data, "stream_channel", this->stream_channel.c_str());
    obs_data_set_string(data, "vod_title", this->vod_title.c_str());
    obs_data_set_string(data, "vod_channel", this->vod_channel.c_str());

    if (!obs_data_save_json_safe(data, path, "tmp", "bak"))
        blog(LOG_WARNING, "Failed to save nudgis_config");

    obs_data_release(data);
}

NudgisConfig * NudgisConfig::GetCurrentNudgisConfig()
{
    static bool first_call = true;
    if (first_call)
    {
        currentNudgisConfig.load();
        first_call = false;
    }
    return &currentNudgisConfig;
}