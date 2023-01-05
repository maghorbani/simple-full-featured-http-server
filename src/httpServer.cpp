#include "httpServer.h"

#include <chrono>
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

  m_address.sin_addr.s_addr = INADDR_ANY;
  m_address.sin_port = port;
  memset(m_address.sin_zero, '\0', sizeof(m_address.sin_zero));
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  std::cout << "socket: " << m_socket << std::endl;
  int new_socket;
  size_t addr_len = sizeof(m_address);
  std::cout << bind(m_socket, (sockaddr *)(&m_address), sizeof(m_address))
            << std::endl;

  ::listen(m_socket, 10);

  while (true) {
    new_socket = accept(m_socket, reinterpret_cast<sockaddr *>(&m_address),
                        (socklen_t *)(&addr_len));
    char buff[1000];
    read(new_socket, buff, 1000);
    printf("%s\n", buff);
    write(new_socket, "Hello", strlen("Hello"));
    close(new_socket);
  }
}
} // namespace http
