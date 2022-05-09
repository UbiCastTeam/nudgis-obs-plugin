#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>
#include <list>
#include <vector>
#include <curl/curl.h>
#include <obs-data.h>

using namespace std;

typedef long http_code_t;

#define HTTP_CODE_UNDEF ((http_code_t)-1)
#define CURL_CODE_UNDEF ((CURLcode)-1)

class HttpClientFormField {
public:
    string name = {};
    string filename = {};
    const char *data = NULL;
    size_t datasize = 0;
};

class HttpClientError {
public:
    string url = {};
    string error = {};
    http_code_t http_code = HTTP_CODE_UNDEF;
    CURLcode curl_code = CURL_CODE_UNDEF;
};

class HttpClient {
public:
    enum HTTP_CLIENT_METHOD {
        HTTP_CLIENT_METHOD_POST,
        HTTP_CLIENT_METHOD_GET,
    };

    enum HTTP_CLIENT_BOOL_ITEM {
        HTTP_CLIENT_BOOL_ITEM_UNDEF,
        HTTP_CLIENT_BOOL_ITEM_TRUE,
        HTTP_CLIENT_BOOL_ITEM_FALSE,
    };

    HttpClient();
    ~HttpClient();

    void reset();
    bool send();

    void setMethod(HTTP_CLIENT_METHOD method);
    void setUrl(const string &url);
    void setParameters(const string &parameters);
    void setHeaders(const vector<string> &headers);
    void setFormFields(const list<HttpClientFormField> &form_fields);

    const HttpClientError &getError() const;
    string &getResponse();
    obs_data_t *getResponseObsData() const;
    bool getSendSuccess() const;

private:
    HTTP_CLIENT_METHOD method;
    HttpClientError error;
    string url;
    string response;
    obs_data_t *response_obs_data;
    string parameters;
    list<HttpClientFormField> form_fields;
    vector<string> headers;
    HTTP_CLIENT_BOOL_ITEM success_json_item;
    string error_json_item;
    bool send_success;
};

#endif //HTTP_CLIENT_HPP