#include "httpServer.h"
#include "stringTools.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <json.h>
#include <map>
#include <memory>
#include <regex>
#include <thread>
int main(int argc, char *argv[]) {

  std::map<std::string, double> name_to_age;
  try {
    std::cout << "starting the server" << std::endl;
    http::Server server;

    server.get("/persons", [&](http::Request req, http::Response res) {
      std::string nameQuery;
      if (req.getQueryMap().contains("name"))
        nameQuery = req.getQueryMap()["name"];

      std::regex reg{".*" + nameQuery + ".*", std::regex_constants::ECMAScript |
                                                  std::regex_constants::icase};

      http::json resJson = http::json::array();
      for (auto const &[name, age] : name_to_age) {
        if (std::regex_search(name, reg)) {
          http::json item = http::json::object();
          item["name"] = name;
          item["age"] = age;
          resJson.push_back(item);
        }
      }
      res.json(resJson);
    });

    server.post("/persons", [&](http::Request req, http::Response res) {
      if (req.getContentType() != "application/json") {
        return res.status(400).write("body must be a json");
      }
      http::json reqJson = req.getJson();
      if (!reqJson.isObject()) {
        return res.status(400).write("body must be an object json");
      }
      if (!reqJson.has("name") || !reqJson.has("age")) {
        return res.status(400).write(
            "bad request, json must contain name and age keys");
      }
      std::string name;
      try {
        name = reqJson["name"].getString();
        http::string::tolower(&name);
      } catch (std::bad_optional_access &e) {
        return res.status(400).write("bad request, name must be a string");
      }
      double age;
      if (reqJson["age"].isDouble())
        age = reqJson["age"].getDouble();
      else if (reqJson["age"].isInt())
        age = static_cast<double>(reqJson["age"].getInt());
      else
        return res.status(400).write("bad request, age must be a number");

      name_to_age[name] = age;

      http::json resJson = http::json::object();
      resJson["message"] = "person successfully added";
      res.json(resJson);
    });

    server.get("*", [](http::Request req, http::Response res) {
      res.html("<h1>Not Found</h1>");
    });
    server.listen(8080);

    int c;
    while (true) {
      c = getchar();
      if (c == 27) {
        break;
      }
    }
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
}