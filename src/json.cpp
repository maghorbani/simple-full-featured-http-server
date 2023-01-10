#include "json.h"
#include <memory>
#include <optional>
#include <queue>
#include <regex>
#include <stack>
#include <stringTools.h>
namespace http {

json::json() : m_type(type_null) {}

json::json(json &j) {
  m_type = j.m_type;
  switch (j.m_type) {
  case type_string:
    val_str = j.val_str;
    break;
  case type_int:
    val_int = j.val_int;
    break;
  case type_double:
    val_double = j.val_double;
    break;
  case type_boolean:
    val_bool = j.val_bool;
    break;

  case type_object:
    val_obj = j.val_obj;

  case type_array:
    val_array = j.val_array;

  default:
    break;
  }
}

// json::json(json &&j) {
//   m_type = j.m_type;
//   switch (j.m_type) {
//   case type_string:
//     val_str = j.val_str;
//     break;
//   case type_int:
//     val_int = j.val_int;
//     break;
//   case type_double:
//     val_double = j.val_double;
//     break;
//   case type_boolean:
//     val_bool = j.val_bool;
//     break;

//   case type_object:
//     val_obj = std::move(j.val_obj);

//   case type_array:
//     val_array = std::move(j.val_array);

//   default:
//     break;
//   }
// }

json::json(double d) {
  m_type = type_double;
  val_double = d;
}

json::json(int64_t i) {
  m_type = type_int;
  val_int = i;
}

json::json(std::string s) {
  m_type = type_string;
  val_str = s;
}
json::json(std::string &&s) {
  m_type = type_string;
  val_str = std::move(s);
}

json::json(const char *ch) {
  m_type = type_string;
  val_str.assign(ch);
}

json::json(bool b) {
  m_type = type_boolean;
  val_bool = b;
}
json::~json() {}

std::string json::getString() {
  if (m_type != type_string)
    throw std::bad_optional_access();
  return val_str;
}

int64_t json::getInt() {

  if (m_type != type_int)

    throw std::bad_optional_access();
  return val_int;
}

double json::getDouble() {

  if (m_type != type_double)
    throw std::bad_optional_access();
  return val_double;
}

bool json::getBool() {
  if (m_type != type_boolean)
    throw std::bad_optional_access();
  return val_bool;
}

std::vector<json::ptr> &json::getArr() {
  if (m_type != type_array)
    throw std::bad_optional_access();
  return val_array;
}
std::map<std::string, json::ptr> &json::getObj() {
  if (m_type != type_object)
    throw std::bad_optional_access();
  return val_obj;
}

size_t json::size() {
  if (m_type != type_array)
    throw std::bad_optional_access();
  return val_array.size();
}

bool json::has(std::string k) {
  if (m_type != type_object)
    throw std::bad_optional_access();
  return val_obj.contains(k);
}

json &json::operator[](const std::string k) {
  if (m_type != type_object)
    throw std::bad_optional_access();
  if (!val_obj.contains(k))
    val_obj[k] = ptr(new json());
  return *val_obj[k];
}

void json::push_back(json &val) {
  if (m_type != type_array)
    throw std::bad_optional_access();

  val_array.push_back(ptr(new json(val)));
}

// void json::push_back(json &&val) {
//   if (m_type != type_array)
//     throw std::bad_optional_access();

//   val_array.push_back(ptr(new json(std::move(val))));
// }

json &json::operator[](size_t ind) {
  if (m_type != type_array)
    throw std::bad_optional_access();

  if (ind >= val_array.size()) {
    throw std::out_of_range("out of range");
  }
  return *val_array[ind];
}

json::ptr json::parse(std::string &&s) {

  std::string tmp{std::move(s)};
  return json::parse(tmp);
}

json::ptr json::parse(std::string &s) {

  string::trim(&s);
  std::stack<json::ptr> jsonStack{};
  std::stack<std::pair<size_t, char>> tokenStack{};
  std::pair<std::string, size_t> current_ind;

  std::string::size_type pos;
  std::string tmp;
  std::string key;
  json::ptr current;
  bool done = false;
  for (std::string::size_type i{}; i < s.length(); i++) {

    if (tokenStack.size()) {

      if (tokenStack.top().second != '"') {
        i = s.find_first_not_of(" \n\r\t\f\v", i);
      } else if (s[i] != '"' || s[i - 1] == '\\') {
        continue;
      }
    }

    if (done) {
      if (i != std::string::npos) {
        throw std::invalid_argument("json not valid");
      }
    }
    if (tokenStack.size() && jsonStack.size()) {
      if (jsonStack.top()->m_type == type_object &&
          (tokenStack.top().second == ',' || tokenStack.top().second == '{') &&
          s[i] != '"' && s[i] != ':' && s[i] != '}')
        throw std::invalid_argument("json not valid");
    }

    switch (s[i]) {
    case '{':
      if (tokenStack.size()) {
        if (tokenStack.top().second == '}' || tokenStack.top().second == ']')
          throw std::invalid_argument("json not valid");
      }

      current = ptr(new json());
      current->m_type = type_object;
      break;
    case '[':
      if (tokenStack.size()) {
        if (tokenStack.top().second == '}' || tokenStack.top().second == ']')
          throw std::invalid_argument("json not valid");
      }
      current = ptr(new json());
      current->m_type = type_array;
      break;
    case '}':
      current = jsonStack.top();
      jsonStack.pop();
      if (current->m_type != type_object) {
        throw std::invalid_argument("json not valid");
      }
      tokenStack.push({i, s[i]});
      if (jsonStack.empty())
        done = true;
      break;
    case ']':
      current = jsonStack.top();
      jsonStack.pop();
      if (current->m_type != type_array) {
        throw std::invalid_argument("json not valid");
      }
      tokenStack.push({i, s[i]});
      if (jsonStack.empty())
        done = true;
      break;

    case '"':
      if (tokenStack.size() && tokenStack.top().second == '"') {
        pos = tokenStack.top().first;
        tokenStack.pop();
        tmp.assign(s.begin() + pos + 1, s.begin() + i);

        current = ptr(new json());
        current->m_type = type_string;
        current->val_str.assign(tmp);
        if (jsonStack.empty()) {
          done = true;
          continue;
        }
        if (jsonStack.top()->m_type == type_array) {
          jsonStack.top()->val_array.push_back(current);
        } else if (jsonStack.top()->m_type == type_object) {

          if (tokenStack.top().second == '{' ||
              tokenStack.top().second == ',') {
            key.assign(tmp);
          } else {
            jsonStack.top()->val_obj[key] = current;
          }
        }
      } else {
        tokenStack.push({i, s[i]});
      }
      break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      pos = s.find_first_not_of("0123456789.", i);
      if (pos == i + 1 && s[i] == '-') {
        throw std::invalid_argument("json not valid");
      }
      if (pos == std::string::npos && jsonStack.size()) {
        throw std::invalid_argument("json not valid");
      }
      if (pos == std::string::npos)
        tmp.assign(s.begin() + i, s.end());
      else
        tmp.assign(s.begin() + i, s.begin() + pos);
      current = ptr(new json());
      current->setPrimitiveFromString(tmp);
      if (jsonStack.empty()) {
        done = true;
        i = pos - 1;
        continue;
      } else if (jsonStack.top()->m_type == type_array) {
        jsonStack.top()->val_array.push_back(current);
      } else if (jsonStack.top()->m_type == type_object) {
        jsonStack.top()->val_obj[key] = current;
      }
      i = pos - 1;

      break;

    case 'n':
    case 't':
      pos = i + 4;
      if (s.size() < pos) {
        throw std::invalid_argument("json not valid");
      }
      tmp.assign(s.begin() + i, s.begin() + pos);
      string::tolower(&tmp);
      current = ptr(new json());
      break;
    case 'f':
      pos = i + 5;
      if (s.size() < pos) {
        throw std::invalid_argument("json not valid");
      }
      tmp.assign(s.begin() + i, s.begin() + pos);
      string::tolower(&tmp);
      current = ptr(new json());
      break;
    case ':':
    case ',':
      tokenStack.push({i, s[i]});
      break;
    default:
      throw std::invalid_argument("json not valid");
      break;
    }

    if (s[i] == '{' || s[i] == '[') {
      if (jsonStack.size()) {
        if (jsonStack.top()->m_type == type_array &&
            (tokenStack.top().second == '[' ||
             tokenStack.top().second == ',')) {
          jsonStack.top()->val_array.push_back(current);
        } else if (jsonStack.top()->m_type == type_array &&
                   tokenStack.top().second == ':') {
          jsonStack.top()->val_obj[key] = current;
        } else {
          throw std::invalid_argument("json not valid");
        }
      }
      tokenStack.push({i, s[i]});
      jsonStack.push(current);
    }
    if (s[i] == 'n') {
      if (tmp != "null") {
        throw std::invalid_argument("json not valid");
      }
      current->m_type = type_null;
    }
    if (s[i] == 't') {
      if (tmp != "true") {
        throw std::invalid_argument("json not valid");
      }
      current->m_type = type_boolean;
      current->val_bool = true;
    }
    if (s[i] == 'f') {
      if (tmp != "false") {
        throw std::invalid_argument("json not valid");
      }
      current->m_type = type_boolean;
      current->val_bool = false;
    }
    if (s[i] == 'n' || s[i] == 't' || s[i] == 'f') {
      if (jsonStack.empty()) {
        break;
      } else if (jsonStack.top()->m_type == type_array) {
        jsonStack.top()->val_array.push_back(current);
      } else if (jsonStack.top()->m_type == type_object) {
        jsonStack.top()->val_obj[key] = current;
      }
      i = pos - 1;
    }
  }

  return current;
}

std::string json::toString() {
  if (m_type == type_array) {
    std::string out{"["};
    for (size_t i{}; i < val_array.size(); i++) {
      if (i > 0) {
        out += ",";
      }
      out += val_array[i]->toString();
    }
    out += "]";
    return out;
  }
  if (m_type == type_object) {
    std::string out{"{"};
    size_t i{};
    for (auto itr = val_obj.begin(); itr != val_obj.end(); itr++) {
      if (i > 0) {
        out += ",";
      }
      out += "\"" + itr->first + "\":";
      out += itr->second->toString();
      i++;
    }
    out += "}";
    return out;
  }
  if (m_type == type_string)
    return std::string("\"" + val_str + "\"");
  if (m_type == type_int)
    return std::to_string(val_int);
  if (m_type == type_double)
    return std::to_string(val_double);
  if (m_type == type_boolean)
    return val_bool ? "true" : "false";
  if (m_type == type_null)
    return "null";

  return "";
}

void json::setPrimitiveFromString(std::string str) {
  string::trim(&str);
  if (str.size() == 0) {
    m_type = type_null;
    return;
  }
  char *endptr;
  int64_t _tmp_int = strtoll(str.c_str(), &endptr, 10);
  if (*endptr == '\0') {
    m_type = type_int;
    val_int = _tmp_int;
    return;
  }
  double _tmp_double = strtod(str.c_str(), &endptr);
  if (*endptr == '\0') {
    m_type = type_double;
    val_double = _tmp_double;
    return;
  }

  if (str == "true") {
    m_type = type_boolean;
    val_bool = true;
    return;
  }
  if (str == "false") {
    m_type = type_boolean;
    val_bool = false;
    return;
  }
  if (str.front() == '\"' && str.back() == '\"') {
    m_type = type_string;
    val_str.assign(str.begin() + 1, str.end() - 1);
    return;
  }

  throw std::invalid_argument(
      std::string("invalid json value: " + str).c_str());
}
} // namespace http