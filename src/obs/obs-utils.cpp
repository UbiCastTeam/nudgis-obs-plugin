#include "obs-utils.hpp"

#include <sstream>
#include <string>
#include <obs-frontend-api.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

using namespace std;

const char *obs_frontend_get_current_profile_path(const char *filename)
{
    static string result;

    char *path = obs_frontend_get_current_profile_path();
    if (path != NULL) {
        ostringstream current_profile_filename;
        current_profile_filename << path << PATH_SEPARATOR << filename;
        result = current_profile_filename.str();
        bfree(path);
    } else
        result = filename;
    return result.c_str();
}
