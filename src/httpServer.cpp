#include "httpServer.h"

#include <chrono>
#include <exception>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace http {

void Server::listen(uint16_t port) {

  bzero((char *)&m_address, sizeof(m_address));
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  m_address.sin_addr.s_addr = INADDR_ANY;
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0) {
    throw "error creating socket";
  }
  int res = ::bind(m_socket, reinterpret_cast<sockaddr *>(&m_address),
                   static_cast<socklen_t>(sizeof(m_address)));

  if (res != 0) {
    throw "error binding";
  }
  res = ::listen(m_socket, 10000);
  if (res != 0) {
    throw "error listening";
  }
  int new_socket;
  std::string html = "<h1>Hello World</h1>";
  std::string resp = "HTTP/1.1 200 OK\nContent-Type: "
                     "text/html;charset=UTF-8\nContent-Length:" +
                     std::to_string(html.length()) + "\n\n" + html;
  while (true) {
    new_socket = accept(m_socket, nullptr, nullptr);
    char buff[1000];
    read(new_socket, buff, 1000);
    write(new_socket, resp.c_str(), resp.length());
    close(new_socket);
  }
}
} // namespace http
