
#include "httpServer.h"
#include "httpRequest.h"
#include "json.h"
#include "stringTools.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
namespace http {
void Server::threadWorker(std::mutex *_mutex, std::mutex *_map) {
  while (true) {
    {
      _mutex->lock();
      if (client_sockets.empty()) {
        std::unique_lock<std::mutex> _lk{*_map};
        if (client_sockets.empty())
          cv.wait_for(_lk, std::chrono::milliseconds(10));
        if (client_sockets.empty()) {
          _mutex->unlock();
          continue;
        }
      }
      _mutex->unlock();
    }

    if (!alive)
      break;

    _map->lock();
    if (client_sockets.empty()) {
      _map->unlock();
      continue;
    }
    int current{std::move(client_sockets.front())};
    client_sockets.pop();
    _map->unlock();

    if (current <= 0)
      continue;

    Request req;
    Response res(current, &m_socket_mutex);

    try {
      read(current, &req);
    } catch (std::length_error &e) {
      if (e.what() == "large payload")
        try {
          res.status(400).write(e.what());
        } catch (...) {
        }
      close(current);
      continue;
    } catch (std::exception &e) {
      try {
        res.status(400).write(e.what());
      } catch (...) {
      }
      close(current);
      continue;
    } catch (...) {
      try {
        res.status(500).write("Internal Server Error");
      } catch (...) {
      }
      close(current);
      continue;
    }
    RequestHandler h;

    try {
      h = m_pathToHandlerMap.at({req.getPath(), req.getMethod()});
    } catch (std::out_of_range &e) {

      try {
        h = m_pathToHandlerMap.at({"*", req.getMethod()});
      } catch (...) {
        res.status(404).write("not found");
        continue;
      }
    } catch (...) {
      try {
        res.status(500).write("Internal Server Error");
      } catch (...) {
      }
      close(current);
      continue;
    }
    try {
      h(req, res);
    } catch (std::exception &e) {
      try {
        res.status(400).write(e.what());
      } catch (...) {
      }
      close(current);
      continue;
    } catch (...) {
      try {
        res.status(500).write("Internal Server Error");
      } catch (...) {
      }
      close(current);
      continue;
    }

    close(current);
  }
}

void Server::read(int _fd, Request *req) {

  // waiting for the socket to be ready to read from
  fd_set _fd_set;
  FD_ZERO(&_fd_set);
  FD_SET(_fd, &_fd_set);
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 100000;
  int res = select(_fd + 1, &_fd_set, nullptr, nullptr, &tv);
  // if it was not ready after a while, properly is was an empty message and we
  // discard it
  if (res <= 0)
    throw std::length_error("empty message");

  // reading the whole message in chunks of size 1000 bytes
  // but there is a limit of 15000 bytes
  std::string message;
  size_t ind = 0, tmp_ind = -1;
  while (true) {
    if (tmp_ind == 0 || tmp_ind < 2048)
      break;
    if (ind >= 15000) {
      throw std::length_error("large payload");
    }
    message.append(2048, 0);

    tmp_ind = ::read(_fd, &message[ind], 2048);

    if (tmp_ind > 2048) {
      throw std::length_error("bad payload");
    }
    ind += tmp_ind;
    if (tmp_ind != 0)
      message.erase(ind);
  }
  if (tmp_ind == 0 && ind == 0) {
    throw std::length_error("empty payload");
  }

  // spliting the message with newlines
  // and continuing it till we reach an empty line.
  bool found{false};
  std::string::size_type pos{};
  std::string::size_type last_pos{};
  std::string tmp;

  do {
    pos = message.find("\n", last_pos);

    found = pos != std::string::npos;
    if (found) {
      tmp.assign(message.begin() + last_pos, message.begin() + pos);
      string::trim(&tmp);
      if (tmp.size())
        if (last_pos == 0)
          req->setMethodAndPath(tmp);
        else
          req->setHeader(tmp);
      else {
        break;
      }
      last_pos = pos + 1;
    } else if (last_pos) {
      tmp.assign(message.begin() + last_pos, message.end());
      string::trim(&tmp);
      if (tmp.size())
        req->setHeader(tmp);
    } else {
      req->setMethodAndPath(message);
      return;
    }
  } while (found);

  // so header is ended, reading body and setting it to request instance
  req->setBody(std::string(message, pos + 1, req->getContentLength()));
}

Server::Server() { alive = true; }

Server::~Server() {
  std::cout << "~Server" << std::endl;
  {
    std::lock_guard<std::mutex> guard(this->m_acceptor_mutex);
    alive = false;
  }
  cv.notify_all();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  for (std::mutex *m : m_responder_mutexes) {
    std::lock_guard<std::mutex> lk(*m);
    delete m;
  }
  if (shutdown(m_socket, SHUT_RDWR) < 0) {
    std::cout << "error shutting down";
  }
  std::cout << "close: " << close(m_socket) << std::endl;
}

void Server::listen(uint16_t port, uint8_t j) {

  bzero((char *)&m_address, sizeof(m_address));
  m_address.sin_family = AF_INET;
  m_address.sin_port = htons(port);
  m_address.sin_addr.s_addr = INADDR_ANY;
  m_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (m_socket < 0) {
    throw std::runtime_error("error creating socket");
  }
  int yes = 1;
  if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    throw std::runtime_error("error setsockopt");
  }

  int res = ::bind(m_socket, reinterpret_cast<sockaddr *>(&m_address),
                   static_cast<socklen_t>(sizeof(m_address)));

  if (res != 0) {
    throw std::runtime_error(
        std::string("error binding " + std::to_string(res)).c_str());
  }
  res = ::listen(m_socket, 10000);
  if (res != 0) {
    throw std::runtime_error("error listening");
  }
  int new_socket;

  if (j == 255) {
    j = std::thread::hardware_concurrency() / 2;
  }
  if (j == 0) {
    j = 1;
  }

  for (int i{}; i < j; i++) {
    m_responder_mutexes.push_back(new std::mutex());
    std::thread t(
        [&]() { threadWorker(m_responder_mutexes.back(), &m_map_mutex); });
    t.detach();
  }

  std::thread acceptor([&]() {
    struct timeval tv;

    while (true) {
      {
        std::lock_guard<std::mutex> guard(this->m_acceptor_mutex);
        if (!alive)
          break;
      }
      fd_set rdfs;

      tv.tv_sec = 0;
      tv.tv_usec = 10000;
      FD_ZERO(&rdfs);
      FD_SET(m_socket, &rdfs);
      int r = select(m_socket + 1, &rdfs, nullptr, nullptr, &tv);
      if (r == -1) {
        throw std::string("error in select");
      }
      if (r) {

        new_socket = accept(m_socket, nullptr, nullptr);

        if (new_socket <= 0) {
          std::cout << "accept error" << std::endl;
          continue;
        }

        m_map_mutex.lock();
        client_sockets.push(new_socket);
        m_map_mutex.unlock();
        cv.notify_one();
      }
    }
  });

  acceptor.detach();

  std::cout << "listening on port " << port << " in " << static_cast<int>(j)
            << " threads" << std::endl;
}

void Server::get(std::string p, RequestHandler h) {
  listener(p, Request::GET, h);
}
void Server::post(std::string p, RequestHandler h) {
  listener(p, Request::POST, h);
}
void Server::put(std::string p, RequestHandler h) {
  listener(p, Request::PUT, h);
}
void Server::patch(std::string p, RequestHandler h) {
  listener(p, Request::PATCH, h);
}
void Server::del(std::string p, RequestHandler h) {
  listener(p, Request::DELETE, h);
}
void Server::listener(std::string path, Request::method m, RequestHandler &h) {
  string::trim(&path);
  string::tolower(&path);
  if (m_pathToHandlerMap.contains({path, m})) {
    throw std::invalid_argument("path already exists");
  }
  m_pathToHandlerMap[{path, m}] = h;
}
} // namespace http
