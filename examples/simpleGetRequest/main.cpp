#include "httpServer.h"
#include <iostream>

int main(int argc, char *argv[]) {
  std::cout << "starting the server" << std::endl;
  http::Server server;
  server.listen();
}