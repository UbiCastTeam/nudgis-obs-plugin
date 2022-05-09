#include "http-client.hpp"

#define mlog(level, msg, ...) blog(level, "[http-client] " msg, ##__VA_ARGS__)

#include <obs-module.h>

static size_t curl_write_function(char *buffer, size_t size, size_t nitems, string *response);

static const char * HTTP_CLIENT_METHOD_STR[]=
{
    [HttpClient::HTTP_CLIENT_METHOD_POST] = "POST",
    [HttpClient::HTTP_CLIENT_METHOD_GET] = "GET",
};

static const char * HTTP_CLIENT_BOOL_ITEM_STR[]=
{
    [HttpClient::HTTP_CLIENT_BOOL_ITEM_UNDEF] = "UNDEF",
    [HttpClient::HTTP_CLIENT_BOOL_ITEM_TRUE] = "TRUE",
    [HttpClient::HTTP_CLIENT_BOOL_ITEM_FALSE] = "FALSE",
};

HttpClient::HttpClient()
{
     reset();
     this->response_obs_data = NULL;
}

HttpClient::~HttpClient()
{
    if (this->response_obs_data != NULL)
    {
        obs_data_release(this->response_obs_data);
        this->response_obs_data = NULL;
    }
}

void HttpClient::reset()
{
    this->method = HTTP_CLIENT_METHOD_POST;
    this->url = {};
    this->parameters = {};
    this->headers = {};
    this->form_fields = {};
}

void HttpClient::setMethod(HTTP_CLIENT_METHOD method)
{
    this->method = method;
}

void HttpClient::setUrl(const string & url)
{
    this->url = url;
}

void HttpClient::setParameters(const string & parameters)
{
    this->parameters = parameters;
}


void HttpClient::setHeaders(const vector<string> & headers)
{
    this->headers = headers;
}

void HttpClient::setFormFields(const list<HttpClientFormField> & form_fields)
{
    this->form_fields = form_fields;
}

size_t curl_write_function(char *buffer, size_t size, size_t nitems, string *response)
{
    size_t result = size * nitems;

    if (result)
        response->append(buffer, result);

    return result;
}

bool HttpClient::send()
{
    this->send_success = false;
    CURL *curl;
    char error_buffer[CURL_ERROR_SIZE];
    struct curl_slist *headers = NULL;
    curl_mime *form = NULL;
    curl_mimepart *field = NULL;

    this->error = {};
    this->response = {};
    this->error_json_item = {};
    this->success_json_item = HTTP_CLIENT_BOOL_ITEM_UNDEF;
    if (this->response_obs_data != NULL)
    {
        obs_data_release(this->response_obs_data);
        this->response_obs_data = NULL;
    }

    curl = curl_easy_init();
    if (curl)
    {
        this->error.url = this->url;
        string url = this->url;
        error_buffer[0] = '\0';

        if (this->method == HTTP_CLIENT_METHOD_GET && !this->parameters.empty())
            url += "?" + this->parameters;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_function);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &this->response);

        if (this->method == HTTP_CLIENT_METHOD_POST)
        {
            curl_easy_setopt(curl, CURLOPT_POST, 1);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, this->parameters.c_str());
        }

        for (std::string &header : this->headers)
            headers = curl_slist_append(headers, header.c_str());

        if (headers != NULL)
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        if (this->form_fields.size() > 0)
            form = curl_mime_init(curl);

        for (HttpClientFormField &form_field : this->form_fields)
        {
            field = curl_mime_addpart(form);
            curl_mime_name(field, form_field.name.c_str());
            if (form_field.filename.length() > 0)
                curl_mime_filename(field, form_field.filename.c_str());
            if (form_field.data != NULL)
                curl_mime_data(field, form_field.data, form_field.datasize > 0 ? form_field.datasize : CURL_ZERO_TERMINATED);
        }

        if (this->form_fields.size() > 0)
            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);

        mlog(LOG_DEBUG, "try to send http request to %s (method: %s)", url.c_str(), HTTP_CLIENT_METHOD_STR[this->method]);
        this->error.curl_code = curl_easy_perform(curl);
        mlog(LOG_DEBUG, "  curl_code           : %d", this->error.curl_code);

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &this->error.http_code);
        mlog(LOG_DEBUG, "  http_code           : %ld", this->error.http_code);

        mlog(LOG_DEBUG, "  response            : %s", response.c_str());

        if (this->error.curl_code == CURLE_OK)
        {
            this->response_obs_data = obs_data_create_from_json(this->response.c_str());
            mlog(LOG_DEBUG, "  response is         : %s", this->response_obs_data ? "json" : "not json");

            if (this->response_obs_data)
            {
                bool has_success_item = obs_data_has_user_value(this->response_obs_data, "success");
                mlog(LOG_DEBUG, "  has_success_item    : %s", has_success_item ? "yes" : "no");
                if (has_success_item)
                    this->success_json_item = obs_data_get_bool(this->response_obs_data, "success") ? HTTP_CLIENT_BOOL_ITEM_TRUE : HTTP_CLIENT_BOOL_ITEM_FALSE;

                bool has_error_item = obs_data_has_user_value(this->response_obs_data, "error");
                mlog(LOG_DEBUG, "  has_error_item      : %s", has_error_item ? "yes" : "no");
                if (has_error_item)
                    this->error_json_item = obs_data_get_string(this->response_obs_data, "error");
            }
        }

        mlog(LOG_DEBUG, "  success_json_item   : %s", HTTP_CLIENT_BOOL_ITEM_STR[this->success_json_item]);
        mlog(LOG_DEBUG, "  error_json_item     : %s", this->error_json_item.c_str());

        if (this->error.curl_code == CURLE_OK && this->error.http_code == 200 && this->success_json_item != HTTP_CLIENT_BOOL_ITEM_FALSE && this->error_json_item.empty())
            this->send_success = true;
        else if (this->error.curl_code != CURLE_OK)
            this->error.error = error_buffer;
        else if (!this->error_json_item.empty())
            this->error.error = this->error_json_item;
        else
            this->error.error = this->response;

        if (form != NULL)
            curl_mime_free(form);

        if (headers != NULL)
            curl_slist_free_all(headers);

        curl_easy_cleanup(curl);
    }

    mlog(LOG_DEBUG, "  errorStr            : %s", this->error.error.c_str());
    mlog(LOG_DEBUG, "  send_success        : %s", this->send_success ? "TRUE" : "FALSE");
    return this->send_success;
}

const HttpClientError & HttpClient::getError() const
{
    return this->error;
}

string & HttpClient::getResponse()
{
    return this->response;
}

obs_data_t * HttpClient::getResponseObsData() const
{
    return this->response_obs_data;
}

bool HttpClient::getSendSuccess() const
{
    return this->send_success;
}
