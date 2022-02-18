#ifndef NUDGIS_CONFIG_HPP
#define NUDGIS_CONFIG_HPP

#include <string>
using namespace std;

#define DEF_FILE_NAME "nudgis_config.json"

class NudgisConfig {
public:
    string url;
    string api_key;
    string stream_title;
    string stream_channel;
    const char * auto_delete_uploaded_file;
    const char * publish_recording_automatically;
    NudgisConfig();
    void load(const char *filename = DEF_FILE_NAME);
    void save(const char *filename = DEF_FILE_NAME);
    static NudgisConfig *GetCurrentNudgisConfig();
    static const char** GetAllAutoStates();
    static size_t GetAutoStatesCount();
    const char * FindAutoState(const char *str);
};

#endif