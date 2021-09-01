#ifndef NUDGIS_CONFIG_HPP
#define NUDGIS_CONFIG_HPP

#include <string>
using namespace std;

#define DEF_FILE_NAME      "nudgis_config.json"

class NudgisConfig
{
    public:        
        string url;
        string api_key;        
        string stream_title;
        string stream_channel;
        string vod_title;
        string vod_channel;
        NudgisConfig();
        void load(const char * filename = DEF_FILE_NAME);
        void save(const char * filename = DEF_FILE_NAME);
        static NudgisConfig * GetCurrentNudgisConfig();

    private:
        void load_default();
};

#endif