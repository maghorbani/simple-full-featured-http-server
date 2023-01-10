#include "helpers.h"
#include "httpServer.h"
#include "stringTools.h"
#include <algorithm>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <json.h>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>

inline bool fileExists(const std::string &path) {
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    std::cout << "usage: ./fileServer path [port]" << std::endl;
    return 1;
  }

  std::string dir{argv[1]};
  dir = fs::string::trim(dir);

  if (dir[0] == '.') {
    dir = std::filesystem::absolute(dir);
  }
  uint32_t port = 8080;
  if (argc > 2) {
    char *endptr;
    port = strtoul(argv[2], &endptr, 10);
    if (*endptr != '\0') {
      std::cout << "port should be a number" << std::endl;
      return 1;
    }
  }

  try {
    std::cout << "starting the server, " << std::endl;
    http::Server server;

    server.get("*", [&](http::Request req, http::Response res) {
      std::string path = req.getPathNotLowered();
      if (path == "/")
        path = "/index.html";
      path = "/" + fs::path::concat(dir, path);
      if (fileExists(path)) {
        std::ifstream file(path, std::ios::binary);
        std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
        std::string ext = fs::path::ext(path);
        res.contentType(fs::string::contentTypeFromExt(ext))
            .write(&buffer[0], buffer.size());
      } else {
        res.status(404).html("<h1>file not found</h1>");
      }
    });
    server.listen(port);

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