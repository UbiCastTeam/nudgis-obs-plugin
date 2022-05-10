#ifndef NUDGIS_CONFIG_HPP
#define NUDGIS_CONFIG_HPP

#include <string>
#include <vector>

#define DEF_FILE_NAME "nudgis_config.json"
#define DEF_STREAM_CHANNEL "Personal channel"

class AutoState {
public:
    enum Types {
        AUTOSTATE_ASK,
        AUTOSTATE_NEVER,
        AUTOSTATE_YES,
    };

    std::string name;
    Types type;

    static const std::vector<AutoState> &GetAll();
    static const AutoState &Find(const char *str);
};

class NudgisConfig {
public:
    std::string url;
    std::string api_key;
    std::string stream_title;
    std::string stream_channel;
    const AutoState *auto_delete_uploaded_file;
    const AutoState *publish_recording_automatically;
    uint64_t upload_chunk_size;
    NudgisConfig();
    void load(const char *filename = DEF_FILE_NAME);
    void save(const char *filename = DEF_FILE_NAME);
    static NudgisConfig *GetCurrentNudgisConfig(const char *filename = DEF_FILE_NAME);
};

#endif
