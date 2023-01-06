#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <queue>
#include <stdio.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
namespace http {
class Server {
  int m_socket;
  sockaddr_in m_address;
  std::queue<int> client_sockets;
  std::mutex client_sockets_mutex;
  bool alive;
  std::string resp;
  std::condition_variable cv;

  void threadWorker();

public:
  Server();
  ~Server() {
    cv.notify_all();
    alive = false;
  };
  void listen(uint16_t);
};
} // namespace http

#endif