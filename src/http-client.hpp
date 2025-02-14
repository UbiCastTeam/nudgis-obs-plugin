#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>
#include <list>
#include <vector>
#include <curl/curl.h>
#include <obs-data.h>

typedef long http_code_t;

#define HTTP_CODE_UNDEF ((http_code_t)-1)

class HttpClientFormField {
public:
	HttpClientFormField(std::string name, std::string filename, const char *data, size_t datasize);
	std::string name = {};
	std::string filename = {};
	const char *data = NULL;
	size_t datasize = 0;
};

class HttpClientError {
public:
	std::string url = {};
	std::string error = {};
	http_code_t http_code = HTTP_CODE_UNDEF;
	CURLcode curl_code = CURLE_OK;
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
	void setUrl(const std::string &url);
	void setParameters(const std::string &parameters);
	void setHeaders(const std::vector<std::string> &headers);
	void setFormFields(const std::list<HttpClientFormField> &form_fields);

	const HttpClientError &getError() const;
	std::string &getResponse();
	obs_data_t *getResponseObsData() const;
	bool getSendSuccess() const;

private:
	HTTP_CLIENT_METHOD method;
	HttpClientError error;
	std::string url;
	std::string response;
	obs_data_t *response_obs_data;
	std::string parameters;
	std::list<HttpClientFormField> form_fields;
	std::vector<std::string> headers;
	HTTP_CLIENT_BOOL_ITEM success_json_item;
	std::string error_json_item;
	bool send_success;
};

#endif //HTTP_CLIENT_HPP
