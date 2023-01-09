#ifndef ___json___
#define ___json___
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
namespace http {
class json {
public:
  typedef std::shared_ptr<json> ptr;

  enum types {
    type_int,
    type_double,
    type_string,
    type_boolean,
    type_array,
    type_object,
    type_null
  };

private:
  std::string m_raw{};

  types m_type;

  int64_t val_int;
  double val_double;
  std::string val_str;
  bool val_bool;
  std::vector<ptr> val_array;
  std::map<std::string, ptr> val_obj;

  void setType(types t) { m_type = t; }
  void clearArray() { val_array.clear(); }
  void clearObj() { val_obj.clear(); }

  void setPrimitiveFromString(std::string);

public:
  json();
  json(json &);
  // json(json &&);
  json(double);
  json(int64_t);
  json(std::string);
  json(std::string &&);
  json(bool);
  ~json();

  types type() { return m_type; }
  bool isInt() { return m_type == type_int; }
  bool isDouble() { return m_type == type_double; }
  bool isString() { return m_type == type_string; }
  bool isBool() { return m_type == type_boolean; }
  bool isArray() { return m_type == type_array; }
  bool isObject() { return m_type == type_object; }

  std::string getString();
  int64_t getInt();
  double getDouble();
  bool getBool();
  std::vector<ptr> &getArr();
  std::map<std::string, ptr> &getObj();

  size_t size();
  bool has(std::string);

  json &operator[](const std::string);
  json &operator[](size_t);

  static json::ptr parse(std::string &&);
  static json::ptr parse(std::string &);

  static json intJson(int64_t i) {
    json out;
    out.m_type = type_int;
    out.val_int = i;
    return out;
  }

  static json object() {
    json out;
    out.m_type = type_object;
    return out;
  }

  std::string toString();
};
} // namespace http
#endif