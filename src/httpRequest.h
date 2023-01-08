#ifndef ___http_Request___
#define ___http_Request___
#include <iostream>
#include <map>
#include <string>

namespace http {
enum method { GET, DELETE, POST, PUT, PATCH };
class Request {
  std::map<std::string, std::string> m_headers;
  method m_method;
  std::string m_path;
  std::string m_contentType;

public:
  Request() {}
  void setHeader(std::string &k, std::string &v) { m_headers[k] = v; }
  void setHeader(std::string &&k, std::string &&v) {
    m_headers[std::move(k)] = std::move(v);
  }
  std::string &getHeader(std::string &k) { return m_headers.at(k); }
  void printHeaders() {
    for (const auto &[k, v] : m_headers) {
      std::cout << k << ": " << v << std::endl;
    }
  }

  void setMethod(method m) { m_method = m; }
  void setMethod(std::string &m) {
    if (m == "GET")
      m_method = GET;
    else if (m == "POST")
      m_method = POST;
    else if (m == "PUT")
      m_method = PUT;
    else if (m == "PATCH")
      m_method = PATCH;
    else if (m == "DELETE")
      m_method = DELETE;
    else {
      throw std::runtime_error("method is not valid");
    }
  }

  method getMethod() { return m_method; }

  void setPath(std::string &p) { m_path = p; }
  std::string getPath() { return m_path; }
  void setContentType(std::string &ct) { m_contentType = ct; }
  std::string getContentType() { return m_contentType; }
};
} // namespace http
#endif