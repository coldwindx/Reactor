#pragma once
#include <string>
#include <map>

using std::string, std::map;

class HttpRequest
{
    friend class HttpConnection;

public:
    HttpRequest() = default;
    HttpRequest(string method, string url, string version, string body)
        : method(method), url(url), version(version), body(body) {}
    ~HttpRequest() = default;
    string getMethod() const { return method; }
    string getUrl() const { return url; }
    string getVersion() const { return version; }
    string getBody() const { return body; }

private:
    string method;
    string url;
    string version;
    map<string, string> headers;
    string body;
};