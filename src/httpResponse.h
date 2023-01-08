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
  void write(const std::string &s) { write(s.c_str(), s.length()); }
  void write(const char *bin, size_t len) {
    std::string resp = "HTTP/1.1 " + std::to_string(m_status) + " " +
                       headerHelpers::statusReason(m_status) +
                       "\n"
                       "Content-Type: " +
                       m_contentType +
                       ";charset=UTF-8\n"
                       "Content-Length: " +
                       std::to_string(len) +
                       "\n"
                       "\n";
    ::write(m_socket, resp.c_str(), resp.length());
    ::write(m_socket, bin, len);
  }
};
} // namespace http
#endif
