#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace http {
class Server {
  int m_socket;
  sockaddr_in m_address;

public:
  void listen(uint16_t);
};
} // namespace http

#endif