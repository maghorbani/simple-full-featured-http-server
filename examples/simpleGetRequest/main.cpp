#include "httpServer.h"
#include "stringTools.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <json.h>
#include <memory>
#include <regex>
#include <thread>
int main(int argc, char *argv[]) {
  try {
    std::cout << "starting the server" << std::endl;
    http::Server server;
    server.get("/",
               [](http::Request req, http::Response res) { res.write("OK"); });
    server.get("/age_by_name", [](http::Request req, http::Response res) {
      http::json reqJson = req.getJson();
      std::string name = reqJson["name"].getString();
      http::string::tolower(&name);

      http::json resJson = http::json::object();
      if (name == "john")
        resJson["age"] = http::json::intJson(35);
      else if (name == "josh")
        resJson["age"] = 1.5;
      res.json(resJson);
    });
    server.get("*", [](http::Request req, http::Response res) {
      res.status(404).html(
          "<div style=\"display:flex; width:100% ;height:100%; "
          "align-items:center; justify-content:center\">"
          "<p style=\"padding:5px 20px; background-color:#ccc; "
          "border-radius:5px\">Not Found</p>"
          "</div>");
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