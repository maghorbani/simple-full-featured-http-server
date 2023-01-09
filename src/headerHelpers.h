#pragma once

#include <iostream>
#include <string>

namespace http {
class headerHelpers {
public:
  static std::string statusReason(int s) {
    switch (s) {
    case 200:
      return "OK";
      break;
    case 404:
      return "Not Found";
      break;
    default:
      return "";
      break;

      return "";
    }
  }
  static std::string generateHttpHeaders(int status, std::string contentType,
                                         size_t len) {
    return std::string("HTTP/1.1 " + std::to_string(status) + " " +
                       statusReason(status) +
                       "\n"
                       "Content-Type: " +
                       contentType +
                       ";charset=UTF-8\n"
                       "Content-Length: " +
                       std::to_string(len) +
                       "\n"
                       "\n");
  }
};
} // namespace http
