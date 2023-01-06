#include "httpServer.h"
#include <iostream>

int main(int argc, char *argv[]) {
  try {

    std::cout << "starting the server" << std::endl;
    http::Server server;
    server.listen(8080);
  } catch (std::exception e) {
    std::cout << e.what() << std::endl;
  }
}