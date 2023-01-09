#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <condition_variable>
#include <functional>
#include <httpRequest.h>
#include <httpResponse.h>
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
  typedef std::function<void(const Request &, const Response &)> RequestHandler;
  std::map<std::string, RequestHandler> m_pathToHandlerMap;
  int m_socket;
  sockaddr_in m_address;
  std::queue<int> client_sockets;
  std::mutex client_sockets_mutex;
  bool alive;
  std::string resp;
  std::string resp404;
  std::condition_variable cv;

  void threadWorker();
  void read(int, Request *);

  std::mutex m_acceptor_mutex;
  std::mutex m_responder_mutex;

public:
  Server();
  ~Server();
  void listen(uint16_t);
  void Get(std::string, RequestHandler);
};
} // namespace http

#endif