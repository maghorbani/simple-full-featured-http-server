#ifndef ___Http_String___
#define ___Http_String___
#include <iostream>
#include <string>
#include <vector>
namespace http {
class string {
public:
  static void split(std::string *str, std::string needle,
                    std::vector<std::string> &out) {
    bool found{false};

    std::string::size_type pos{};
    std::string::size_type last_pos{};

    do {
      pos = str->find(needle, last_pos);

      found = pos != std::string::npos;
      if (found) {
        std::string tmp{str->begin() + last_pos, str->begin() + pos};
        if (tmp.size())
          out.push_back(trim(tmp));
        last_pos = pos + needle.size();
      } else if (last_pos) {
        std::string tmp{str->begin() + last_pos, str->end()};
        if (tmp.size())
          out.push_back(trim(tmp));
      } else {
        out.push_back(trim(*str));
      }
    } while (found);
  }

  static std::wstring ltrim(const std::wstring &s) {
    std::wstring WHITESPACE = L" \n\r\t\f\v";
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::wstring::npos) ? L"" : s.substr(start);
  }
  static std::string ltrim(const std::string &s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
  }

  static std::wstring rtrim(const std::wstring &s) {
    std::wstring WHITESPACE = L" \n\r\t\f\v";
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
  }
  static std::string rtrim(const std::string &s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
  }

  template <typename T> static T trim(const T &s) { return rtrim(ltrim(s)); }

  template <typename T> static void replaceAll(T &str, T find, T replace) {
    size_t pos = str.find(find);
    while (pos != T::npos) {
      str.replace(pos, find.size(), replace);
      pos = str.find(find, pos + replace.size());
    }
  }

  static void tolower(std::string *s) {
    std::transform(s->begin(), s->end(), s->begin(),
                   [](unsigned char c) { return std::tolower(c); });
  }

  static void ltrim(std::string *s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t start = s->find_first_not_of(WHITESPACE);
    s->assign((start == std::string::npos) ? "" : s->substr(start));
  }
  static void rtrim(std::string *s) {
    std::string WHITESPACE = " \n\r\t\f\v";
    size_t end = s->find_last_not_of(WHITESPACE);
    s->assign((end == std::string::npos) ? "" : s->substr(0, end + 1));
  }
  static void trim(std::string *s) {
    ltrim(s);
    rtrim(s);
  }

  static std::string urlEncode(std::string str) {
    std::string new_str = "";
    char c;
    int ic;
    const char *chars = str.c_str();
    char bufHex[10];
    int len = str.length();

    for (int i = 0; i < len; i++) {
      c = chars[i];
      ic = c;
      if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        new_str += c;
      else {
        sprintf(bufHex, "%X", c);
        if (ic < 16)
          new_str += "%0";
        else
          new_str += "%";
        new_str += bufHex;
      }
    }
    return new_str;
  }

  static std::string urlDecode(std::string str) {
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for (i = 0; i < len; i++) {
      if (str[i] != '%') {
        if (str[i] == '+')
          ret += ' ';
        else
          ret += str[i];
      } else {
        sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
        ch = static_cast<char>(ii);
        ret += ch;
        i = i + 2;
      }
    }
    return ret;
  }
};
} // namespace http

#endif
