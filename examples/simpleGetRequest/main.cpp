#include "httpServer.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  std::cout << __cplusplus << std::endl;
  try {

    std::cout << "starting the server" << std::endl;
    http::Server server;
    server.Get("/",
               [](http::Request req, http::Response res) { res.write("OK"); });
    server.listen(8080);

    int c;
    while (true) {
      c = getchar();
      if (c == 27) {
        break;
      }
    }
    std::cout << "ended" << std::endl;

  } catch (std::exception &e) {
    std::cout << "exception: " << e.what() << std::endl;
  }
}