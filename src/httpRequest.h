#ifndef ___http_Request___
#define ___http_Request___
#include "json.h"
#include "stringTools.h"
#include <iostream>
#include <map>
#include <optional>
#include <regex>
#include <string>
namespace http {
class Request {
public:
  enum method { GET, DELETE, POST, PUT, PATCH };
  typedef std::shared_ptr<Request> ptr;

private:
  std::map<std::string, std::string> m_headers;
  method m_method;
  std::string m_path, m_path_not_lowered;
  std::string m_query;
  std::map<std::string, std::string> m_quey_map;
  std::string m_contentType;
  size_t m_contentLength{};
  json::ptr m_json;
  std::map<std::string, std::string> m_form;
  std::string m_multiPartBoundary;

public:
  Request() : m_json(nullptr) {
    m_headers.clear();
    m_path.clear();
    m_contentLength = 0;
    m_contentType.clear();
    m_form.clear();
    m_multiPartBoundary.clear();
    m_query.clear();
    m_quey_map.clear();
    m_path_not_lowered.clear();
  }
  Request(Request &r) {
    m_contentType = r.m_contentType;
    m_contentLength = r.m_contentLength;
    m_method = r.m_method;
    m_path = r.m_path;
    m_headers = r.m_headers;
    m_form = r.m_form;
    m_multiPartBoundary = r.m_multiPartBoundary;
    m_query = r.m_query;
    m_quey_map = r.m_quey_map;
    m_path_not_lowered = r.m_path_not_lowered;
    if (r.m_contentType == "application/json") {
      m_json = r.m_json;
    }
  }
  void setHeader(std::string &k, std::string &v) {
    string::tolower(&k);
    if (k == "content-type") {
      string::tolower(&v);

      std::vector<std::string> tmp;
      string::split(&v, ";", tmp);
      if (tmp[0] == "multipart/form-data") {
        if (tmp.size() != 2) {
          throw std::invalid_argument("content-type is not valid");
        }
        auto pos = tmp[1].find("=");
        if (pos == std::string::npos) {
          throw std::invalid_argument("content-type is not valid");
        }
        m_multiPartBoundary.assign(tmp[1], pos + 1, tmp[1].size() - pos - 1);
        v = tmp[0];
      }

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

  void setQuery(std::string &&q) {
    m_query = q;
    m_query = string::urlDecode(m_query);
    std::vector<std::string> tmp;
    string::split(&m_query, "&", tmp);
    size_t assignment;
    for (std::string &item : tmp) {
      assignment = item.find("=");
      if (assignment != std::string::npos) {
        std::string key{item, 0, assignment};
        std::string val{};
        if (assignment < item.length() - 1)
          val.assign(item, assignment + 1, item.length());
        m_quey_map[key] = val;
      }
    }
  }

  std::map<std::string, std::string> getQueryMap() { return m_quey_map; }
  std::string getQuery() { return m_query; }

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
    string::tolower(&m);
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
    size_t qm = p.find("?", 0);

    m_path.assign(p, 0, qm);
    if (qm != std::string::npos) {
      setQuery(std::string(p, qm + 1, p.size()));
    }
    m_path = string::urlDecode(m_path);
    m_path_not_lowered.assign(m_path);
    string::tolower(&m_path);
  }
  std::string getPath() { return m_path; }
  std::string getPathNotLowered() { return m_path_not_lowered; }

  void setMethodAndPath(std::string &l) {
    // string::tolower(&l);
    std::vector<std::string> v;
    string::split(&l, " ", v);
    if (v.size() != 3)
      throw std::invalid_argument(
          std::string("first line of http request is not valid: ").c_str());

    setMethod(v[0]);
    setPath(v[1]);

    string::tolower(&v[2]);
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

  void addFormItem(std::string &v) {
    std::string::size_type assignment = v.find("=");
    if (assignment == std::string::npos) {
      throw std::runtime_error(
          std::string("form data is not valid: " + v).c_str());
    }
    std::string key{v, 0, assignment};
    std::string val{v, assignment + 1, v.size()};
    // string::trim(&key);
    // string::trim(&val);
    m_form[key] = val;
  }
  void setBody(std::string &&b) {

    if (m_contentType == "application/json") {
      m_json = json::parse(b);
    } else if (m_contentType == "application/x-www-form-urlencoded") {
      std::vector<std::string> tmp;
      b = string::urlDecode(b);
      string::split(&b, "&", tmp);
      for (std::string &item : tmp) {
        addFormItem(item);
      }
    } else if (m_contentType == "multipart/form-data") {
      std::regex reg(".*Content-Disposition:\\s*form-data;\\s*name=\\\"(.*)"
                     "\\\"\\s*(.*)\\s*",
                     std::regex_constants::ECMAScript |
                         std::regex_constants::icase);
      std::string needle{"--" + m_multiPartBoundary};
      std::vector<std::string> tmp;
      string::split(&b, needle, tmp);
      for (std::string &item : tmp) {
        if (item.empty() || item == "--")
          continue;

        std::smatch m;
        if (std::regex_match(item, m, reg)) {
          m_form[m[1]] = m[2];
        }
      }
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