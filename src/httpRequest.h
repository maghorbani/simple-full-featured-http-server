#ifndef ___http_Request___
#define ___http_Request___
#include "json.h"
#include "stringTools.h"
#include <iostream>
#include <map>
#include <optional>
#include <string>
namespace http {
enum method { GET, DELETE, POST, PUT, PATCH };
class Request {
  std::map<std::string, std::string> m_headers;
  method m_method;
  std::string m_path;
  std::string m_contentType;
  size_t m_contentLength{};
  json::ptr m_json;

public:
  Request() : m_json(nullptr) {}
  void setHeader(std::string &k, std::string &v) {
    string::tolower(&k);
    if (k == "content-type") {
      string::tolower(&v);
      setContentType(v);
    }
    if (k == "content-length") {
      setContentLength(v);
    }
    m_headers[k] = v;
  }
  void setHeader(std::string &&k, std::string &&v) {
    std::string _k{std::move(k)};
    std::string _v{std::move(v)};
    setHeader(k, v);
  }

  void setHeader(std::string &v) {
    std::string::size_type colon = v.find(":");
    if (colon == std::string::npos) {
      throw std::runtime_error(
          std::string("request header is not valid at: " + v).c_str());
    }
    std::string key{v, 0, colon};
    std::string val{v, colon + 1, v.size()};
    string::trim(&key);
    string::trim(&val);
    setHeader(key, val);
  }
  std::string &getHeader(std::string &k) { return m_headers.at(k); }
  void printHeaders() {
    for (const auto &[k, v] : m_headers) {
      std::cout << k << ": " << v << std::endl;
    }
  }

  void setMethod(method m) { m_method = m; }
  void setMethod(std::string &m) {
    if (m == "get")
      m_method = GET;
    else if (m == "post")
      m_method = POST;
    else if (m == "put")
      m_method = PUT;
    else if (m == "patch")
      m_method = PATCH;
    else if (m == "delete")
      m_method = DELETE;
    else {
      throw std::runtime_error("method is not valid");
    }
  }

  method getMethod() { return m_method; }

  void setPath(std::string &p) {
    m_path = p;
    string::tolower(&m_path);
  }
  std::string getPath() { return m_path; }

  void setMethodAndPath(std::string &l) {
    string::tolower(&l);
    std::vector<std::string> v;
    string::split(&l, " ", v);
    if (v.size() != 3)
      throw std::invalid_argument(
          std::string("first line of http request is not valid: ").c_str());

    setMethod(v[0]);
    setPath(v[1]);

    if (v[2] != "http/1.1" && v[2] != "http/1.0") {
      throw std::invalid_argument(
          std::string("protocol is not valid, just http/1.1 and http/1.0 is "
                      "supported: " +
                      v[2])
              .c_str());
    }
  }
  void setContentType(std::string &ct) { m_contentType = ct; }
  void setContentType(std::string &&ct) { m_contentType.assign(std::move(ct)); }
  std::string getContentType() { return m_contentType; }
  void setContentLength(std::string &cl) {
    char *endptr;

    m_contentLength = strtoull(cl.c_str(), &endptr, 0);
    if (*endptr != '\0' || cl.length() == 0) {
      throw std::invalid_argument(
          "content-length header is not valid, must be an unsigned int");
    }
  }
  void setContentLength(std::string &&cl) {
    char *endptr;

    m_contentLength = strtoull(cl.c_str(), &endptr, 0);
    if (*endptr != '\0' || cl.length() == 0) {
      throw std::invalid_argument(
          "content-length header is not valid, must be an unsigned int");
    }
  }
  size_t getContentLength() { return m_contentLength; }

  void setBody(std::string &&b) {
    if (m_contentType == "application/json") {
      m_json = json::parse(b);
    }
  }

  json &getJson() {
    if (m_contentType != "application/json")
      throw std::bad_optional_access();
    return *m_json;
  }
};
} // namespace http
#endif