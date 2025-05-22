#pragma once
#include <string>
#include <map>
#include "httpconnection.hpp"

using std::map;
using std::string;

class HttpResponse
{
    friend class HttpConnection;

public:
    HttpResponse() = default;
    ~HttpResponse() = default;
    void setVersion(string version) { this->version = version; }
    void setCode(string code) { this->code = code; }
    void setDescription(string description) { this->description = description; }
    void setBody(string body) { this->body = body; }
    void addHeader(string key, string value) { this->headers[key] = value; }
    string getBody() const { return body; }

private:
    string version;
    string code;
    string description;
    map<string, string> headers;
    string body;
};