#include "httpServer.h"
#include <chrono>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace http {
void Server::threadWorker() {
  while (alive) {
    if (client_sockets.empty()) {
      std::unique_lock lk(client_sockets_mutex);
      cv.wait(lk);
    }
    client_sockets_mutex.lock();
    int current{std::move(client_sockets.front())};
    client_sockets.pop();
    client_sockets_mutex.unlock();

    char buff[1000];
    read(current, buff, 1000);
    write(current, resp.c_str(), resp.length());
    close(current);
  }
}

Server::Server() {
  std::string html =
      "<div style=\"display:flex; width:600px;justify-content:space-between\">"
      "<div style=\"width: 150px; height: 150px;background-color:blue\"></div>"
      "<div style=\"width: 150px; height: 150px;background-color:red\"></div>"
      "</div>";

  resp = "HTTP/1.1 200 OK\nContent-Type: "
         "text/html;charset=UTF-8\nContent-Length:" +
         std::to_string(html.length()) + "\n\n" + html;
  alive = true;
}

void Server::listen(uint16_t port) {

  bzero((char *)&m_address, sizeof(m_address));
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  m_address.sin_addr.s_addr = INADDR_ANY;
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0) {
    throw std::runtime_error("error creating socket");
  }
  int res = ::bind(m_socket, reinterpret_cast<sockaddr *>(&m_address),
                   static_cast<socklen_t>(sizeof(m_address)));

  if (res != 0) {
    throw std::runtime_error("error binding");
  }
  res = ::listen(m_socket, 100);
  if (res != 0) {
    throw std::runtime_error("error listening");
  }
  int new_socket;

  std::thread t([&]() { threadWorker(); });
  t.detach();

  while (true) {
    new_socket = accept(m_socket, nullptr, nullptr);
    client_sockets_mutex.lock();
    client_sockets.push(new_socket);
    client_sockets_mutex.unlock();
    cv.notify_one();
  }
}
} // namespace http
