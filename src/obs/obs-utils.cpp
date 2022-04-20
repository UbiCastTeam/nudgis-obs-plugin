#include "obs-utils.hpp"

#include <sstream>
#include <memory>
#include <obs-frontend-api.h>
#include <util/curl/curl-helper.h>

#if defined(WIN32) || defined(_WIN32)
#define PATH_SEPARATOR "\\"
#else
#define PATH_SEPARATOR "/"
#endif

using namespace std;

static auto curl_deleter = [](CURL *curl) { curl_easy_cleanup(curl); };
using Curl = unique_ptr<CURL, decltype(curl_deleter)>;

class app {
public:
    const char *GetVersionString()
    {
        return obs_get_version_string();
    }
};

inline app *App()
{
    static app _instance;
    return &_instance;
}

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

static size_t string_write(char *ptr, size_t size, size_t nmemb, string &str)
{
    size_t total = size * nmemb;
    if (total)
        str.append(ptr, total);

    return total;
}

static size_t header_write(char *ptr, size_t size, size_t nmemb,
                           vector<string> &list)
{
    string str;

    size_t total = size * nmemb;
    if (total)
        str.append(ptr, total);

    if (str.back() == '\n')
        str.resize(str.size() - 1);
    if (str.back() == '\r')
        str.resize(str.size() - 1);

    list.push_back(std::move(str));
    return total;
}

Curl curl{curl_easy_init(), curl_deleter};

bool GetRemoteFile(const char *url, std::string &str, std::string &error,
                   long *responseCode, const char *contentType,
                   std::string request_type, const char *postData,
                   std::vector<std::string> extraHeaders,
                   std::string *signature, int timeoutSec, bool fail_on_error)
{
    vector<string> header_in_list;
    char error_in[CURL_ERROR_SIZE];
    CURLcode code = CURLE_FAILED_INIT;

    error_in[0] = 0;

    string versionString("User-Agent: obs-basic ");
    versionString += App()->GetVersionString();

    string contentTypeString;
    if (contentType) {
        contentTypeString += "Content-Type: ";
        contentTypeString += contentType;
    }

    //Curl curl{curl_easy_init(), curl_deleter};
    if (curl) {
        struct curl_slist *header = nullptr;

        header = curl_slist_append(header, versionString.c_str());

        if (!contentTypeString.empty()) {
            header = curl_slist_append(header,
                                       contentTypeString.c_str());
        }

        for (std::string &h : extraHeaders)
            header = curl_slist_append(header, h.c_str());

        curl_easy_setopt(curl.get(), CURLOPT_URL, url);
        curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, header);
        curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, error_in);
        if (fail_on_error)
            curl_easy_setopt(curl.get(), CURLOPT_FAILONERROR, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION,
                         string_write);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &str);
        curl_obs_set_revoke_setting(curl.get());

        if (signature) {
            curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION,
                             header_write);
            curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA,
                             &header_in_list);
        }

        if (timeoutSec)
            curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT,
                             timeoutSec);

#if LIBCURL_VERSION_NUM >= 0x072400
        // A lot of servers don't yet support ALPN
        curl_easy_setopt(curl.get(), CURLOPT_SSL_ENABLE_ALPN, 0);
#endif
        if (!request_type.empty()) {
            if (request_type != "GET")
                curl_easy_setopt(curl.get(),
                                 CURLOPT_CUSTOMREQUEST,
                                 request_type.c_str());

            // Special case of "POST"
            if (request_type == "POST") {
                curl_easy_setopt(curl.get(), CURLOPT_POST, 1);
                if (!postData)
                    curl_easy_setopt(curl.get(),
                                     CURLOPT_POSTFIELDS,
                                     "{}");
            }
        }
        if (postData) {
            curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS,
                             postData);
        }

        code = curl_easy_perform(curl.get());
        if (responseCode)
            curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE,
                              responseCode);

        if (code != CURLE_OK) {
            error = error_in;
        } else if (signature) {
            for (string &h : header_in_list) {
                string name = h.substr(0, 13);
                if (name == "X-Signature: ") {
                    *signature = h.substr(13);
                    break;
                }
            }
        }

        curl_slist_free_all(header);
    }

    return code == CURLE_OK;
}