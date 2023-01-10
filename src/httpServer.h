#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include "httpRequest.h"
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
  typedef std::function<void(Request &, Response &)> RequestHandler;
  std::map<std::pair<std::string, Request::method>, RequestHandler>
      m_pathToHandlerMap;
  int m_socket;
  sockaddr_in m_address;
  std::queue<int> client_sockets;
  bool alive;
  std::string resp;
  std::string resp404;
  std::condition_variable cv;

  void threadWorker(std::mutex *, std::mutex *);
  void read(int, Request *);

  std::mutex m_acceptor_mutex;
  std::vector<std::mutex *> m_responder_mutexes;
  std::mutex m_map_mutex;

  std::mutex m_socket_mutex;

public:
  Server();
  ~Server();
  void listen(uint16_t);
  void get(std::string, RequestHandler);
  void post(std::string, RequestHandler);
  void put(std::string, RequestHandler);
  void patch(std::string, RequestHandler);
  void del(std::string, RequestHandler);
  void listener(std::string, Request::method, RequestHandler &);
};
} // namespace http

#endif