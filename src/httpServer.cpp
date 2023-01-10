
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

    Request req;
    Response res(current, &m_socket_mutex);
    // std::thread t([&]() {
    try {
      read(current, &req);
      m_pathToHandlerMap.at({req.getPath(), req.getMethod()})(req, res);
      m_socket_mutex.lock();
      close(current);
      m_socket_mutex.unlock();
    } catch (std::length_error &e) {
      // std::cout << "length: " << e.what() << std::endl;
      if (e.what() == "large payload")
        try {

          res.status(400).write(e.what());
        } catch (...) {
        }
      m_socket_mutex.lock();
      close(current);
      m_socket_mutex.unlock();
    } catch (std::out_of_range &e) {
      // std::cout << req.getPath() << ": 404 error " << e.what() << std::endl;
      try {
        res.status(404).write("not found");
      } catch (...) {
      }
      m_socket_mutex.lock();
      close(current);
      m_socket_mutex.unlock();

    } catch (std::exception &e) {
      // std::cout << "error reading: " << e.what() << std::endl;
      try {
        res.status(400).write(e.what());
      } catch (...) {
      }
      m_socket_mutex.lock();
      close(current);
      m_socket_mutex.unlock();
    } catch (...) {
      // std::cout << "unknown" << std::endl;
      try {
        res.status(500).write("Internal Server Error");
      } catch (...) {
      }
      m_socket_mutex.lock();
      close(current);
      m_socket_mutex.unlock();
    }

    // });

    // t.detach();
  }
}

void Server::read(int _fd, Request *req) {

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

  // std::regex contentTypeRegex("(content-length)( )*(:)( )*([0-9]+)",
  //                             std::regex_constants::ECMAScript |
  //                                 std::regex_constants::icase);
  // std::sregex_iterator regItr(message.begin() + ind, message.end(),
  //                             contentTypeRegex);
  // if (regItr != std::sregex_iterator()) {
  //   std::string line = regItr->str();
  //   std::string::size_type colon = line.find(":");
  //   if (colon == std::string::npos) {
  //     throw std::runtime_error(
  //         std::string("request header is not valid at: " + line).c_str());
  //   }
  //   req->setContentLength(std::string(line.begin() + colon + 1, line.end()));
  // }

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
  if (req->getPath() == "") {
    std::cout << "message: " << message << std::endl;
  }

  req->setBody(std::string(message, pos + 1, req->getContentLength()));
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

  resp404 = "HTTP/1.1 404 Not Found\nContent-Type: "
            "text/plain\nContent-Length:9\n\nNot Found";
  alive = true;
}

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

void Server::listen(uint16_t port) {

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

  for (int i{}; i < 1; i++) {
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

      tv.tv_sec = 1;
      tv.tv_usec = 1000;
      FD_SET(m_socket, &rdfs);
      int r = select(m_socket + 1, &rdfs, nullptr, nullptr, &tv);
      if (r == -1) {
        throw std::string("error in select");
      }
      if (r) {

        m_socket_mutex.lock();
        new_socket = accept(m_socket, nullptr, nullptr);

        m_socket_mutex.unlock();

        m_map_mutex.lock();
        client_sockets.push(new_socket);
        m_map_mutex.unlock();
        cv.notify_one();
      }
    }
  });

  acceptor.detach();
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
