#ifndef NUDGIS_CONFIG_HPP
#define NUDGIS_CONFIG_HPP

#include <string>
#include <vector>
using namespace std;

#define DEF_FILE_NAME "nudgis_config.json"

class AutoState {
public:
    enum Types {
        AUTOSTATE_ASK,
        AUTOSTATE_NEVER,
        AUTOSTATE_YES,
    };

    string name;
    Types type;

    static const vector<AutoState> &GetAll();
    static const AutoState &Find(const char *str);
};

class NudgisConfig {
public:
    string url;
    string api_key;
    string stream_title;
    string stream_channel;
    const AutoState *auto_delete_uploaded_file;
    const AutoState *publish_recording_automatically;
    NudgisConfig();
    void load(const char *filename = DEF_FILE_NAME);
    void save(const char *filename = DEF_FILE_NAME);
    static NudgisConfig *GetCurrentNudgisConfig();
};

#endif