/* Function C ++ utilities for basic obs data */

#pragma once

#include <vector>
#include <string>

const char *obs_frontend_get_current_profile_path(const char *filename);

bool GetRemoteFile(
        const char *url, std::string &str, std::string &error,
        long *responseCode = nullptr, const char *contentType = nullptr,
        std::string request_type = "", const char *postData = nullptr,
        bool keepalive = false,
        std::vector<std::string> extraHeaders = std::vector<std::string>(),
        std::string *signature = nullptr, int timeoutSec = 0,
        bool fail_on_error = true);
