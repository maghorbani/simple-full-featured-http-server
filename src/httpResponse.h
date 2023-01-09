#ifndef ___http_Response___
#define ___http_Response___

#include "headerHelpers.h"
#include <iostream>
#include <unistd.h>
namespace http {
class Response {
  int m_socket;
  std::string m_contentType;
  int m_status = 200;

public:
  Response(int _fd) {
    m_socket = _fd;
    m_contentType = "text/plain";
  }
  Response &contentType(std::string ct) {
    m_contentType = ct;
    return *this;
  }
  Response &status(int s) {
    m_status = s;
    return *this;
  }
  void json(std::string &&s) {
    m_contentType = "application/json";
    write(s.c_str(), s.length());
  }
  void json(json::ptr j) {
    m_contentType = "application/json";
    write(j->toString());
  }
  void json(http::json &j) {
    m_contentType = "application/json";
    write(j.toString());
  }
  void html(std::string &&s) {
    m_contentType = "text/html";
    write(s.c_str(), s.length());
  }
  void write(const std::string &s) { write(s.c_str(), s.length()); }
  void write(const char *bin, size_t len) {
    std::string resp =
        headerHelpers::generateHttpHeaders(m_status, m_contentType, len);
    int h = ::write(m_socket, resp.c_str(), resp.length());
    int r = ::write(m_socket, bin, len);
    if (r == -1 || h == -1) {
      throw std::runtime_error("error writing response");
    }
  }
};
} // namespace http

#endif