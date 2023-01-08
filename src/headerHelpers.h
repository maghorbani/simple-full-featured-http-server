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
};
} // namespace http
