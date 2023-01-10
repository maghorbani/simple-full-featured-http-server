#ifndef __HELPERS__
#define __HELPERS__

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs {
class string {
public:
  static std::wstring toWString(const std::string &str) {
    return std::wstring(str.begin(), str.end());
  }

  static std::string toString(const std::wstring &wstr) {
    return std::string(wstr.begin(), wstr.end());
  }

  static std::vector<std::string> split(std::string &str, std::string needle) {
    bool found{false};
    std::vector<std::string> out{};

    std::string::size_type pos{};
    std::string::size_type last_pos{};

    do {
      pos = str.find(needle, last_pos);

      found = pos != std::string::npos;
      if (found) {
        std::string tmp{str.begin() + last_pos, str.begin() + pos};
        if (tmp.size())
          out.push_back(trim(tmp));
        last_pos = pos + needle.size();
      } else if (last_pos) {
        std::string tmp{str.begin() + last_pos, str.end()};
        if (tmp.size())
          out.push_back(trim(tmp));
      } else {
        out.push_back(trim(str));
      }
    } while (found);

    return out;
  }

  static std::string join(std::vector<std::string> vect, std::string sep) {
    std::string out{""};

    for (size_t i{}; i < vect.size(); i++) {
      if (i > 0)
        out += sep;
      out += vect[i];
    }

    return out;
  }

  static std::string ltrim(const std::string &s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
  }

  static std::string rtrim(const std::string &s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  static std::string trim(const std::string &s) { return rtrim(ltrim(s)); }

  static void replaceAll(std::string &str, std::string find,
                         std::string replace) {
    size_t pos = str.find(find);
    while (pos != std::string::npos) {
      str.replace(pos, find.size(), replace);
      pos = str.find(find, pos + replace.size());
    }
  }

  void removeSpaces(std::string &str) {
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
  }

  static std::string removeTailingSlash(std::string path) {
    size_t pos = path.find_last_of('/');
    if (pos == path.size() - 1) {
      path.erase(pos);
    }
    return path;
  }

  static std::string removeStartingSlash(std::string path) {
    size_t pos = path.find_first_of('/');
    if (pos == 0) {
      path.erase(0, 1);
    }
    return path;
  }

  template <typename T> static T removeTailingBackSlash(T path) {
    size_t pos = path.find_last_of('\\');
    if (pos == path.size() - 1) {
      path.erase(pos);
    }
    return path;
  }

  template <typename T> static T removeStartingBackSlash(T path) {
    size_t pos = path.find_first_of('\\');
    if (pos == 0) {
      path.erase(0, 1);
    }
    return path;
  }

  template <typename T> static T trimSlash(T path) {
    return removeStartingSlash(removeTailingSlash(path));
  }

  template <typename T> static T trimBackSlash(T path) {
    return removeStartingBackSlash(removeTailingBackSlash(path));
  }

  static std::string contentTypeFromExt(std::string &_ext) {
    if (_ext == "html" || _ext == "html")
      return "text/html";
    if (_ext == "css")
      return "text/css";
    if (_ext == "json")
      return "application/json";
    if (_ext == "js" || _ext == "mjs")
      return "text/javascript";
    if (_ext == "txt")
      return "text/plain";
    if (_ext == "bmp")
      return "image/bmp";
    if (_ext == "gif")
      return "image/gif";
    if (_ext == "ico")
      return "image/vnd.microsoft.icon";
    if (_ext == "jpeg" || _ext == "jpg")
      return "image/jpeg";
    if (_ext == "tif" || _ext == "tiff")
      return "image/tiff";
    if (_ext == "png")
      return "image/png";
    if (_ext == "svg")
      return "image/svg+xml";
    if (_ext == "webp")
      return "image/webp";
    if (_ext == "pdf")
      return "application/pdf";
    if (_ext == "tar")
      return "application/x-tar";
    if (_ext == "gz")
      return "application/gzip";
    if (_ext == "zip")
      return "application/zip";
    if (_ext == "7z")
      return "application/x-7z-compressed";
    if (_ext == "webp")
      return "image/webp";

    return "application/octet-stream";
  }
};

class path {
public:
  static bool seperatorIsBackSlash(std::string p) {
    std::string::size_type pos = p.find("\\");
    return pos != std::string::npos;
  }

  static std::string ext(std::string &p) {
    auto pos = p.find_last_of(".");
    return std::string(p.begin() + pos + 1, p.end());
  }
  static std::string concat(std::string p1, std::string p2,
                            std::string sep = "/") {
    p1 = string::removeTailingSlash(
        string::removeTailingBackSlash(string::trim(p1)));
    p2 = string::trimSlash(string::trimBackSlash(string::trim(p2)));

    std::vector<std::string> p1_vec;
    std::vector<std::string> p2_vec;
    if (seperatorIsBackSlash(p1)) {
      p1_vec = string::split(p1, "\\");
    } else {
      p1_vec = string::split(p1, "/");
    }

    if (seperatorIsBackSlash(p2)) {
      p2_vec = string::split(p2, "\\");
    } else {
      p2_vec = string::split(p2, "/");
    }

    p1_vec.insert(p1_vec.end(), p2_vec.begin(), p2_vec.end());
    return string::join(p1_vec, sep);
  }

  static std::string parentDir(std::string p, std::string sep = "\\") {
    std::string _sep = "/";
    if (seperatorIsBackSlash(p))
      _sep = "\\";
    std::vector<std::string> vect = string::split(p, _sep);
    vect.pop_back();
    return string::join(vect, sep);
  }
};
} // namespace fs

#endif