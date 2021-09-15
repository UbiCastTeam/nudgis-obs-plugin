#include "obs-utils.hpp"

#include <sstream>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

using namespace std;

const char *obs_frontend_get_current_profile_path(const char *filename)
{
    static string result;

    ostringstream current_profile_filename;
    char *path = obs_frontend_get_current_profile_path();
    current_profile_filename << path << PATH_SEPARATOR << filename;
    bfree(path);
    result = current_profile_filename.str();
    return result.c_str();
}
