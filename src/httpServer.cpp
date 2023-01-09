
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
void Server::threadWorker() {
  while (true) {
    {
      std::lock_guard<std::mutex> lk{m_responder_mutex};
      if (client_sockets.empty()) {
        std::unique_lock<std::mutex> lk{client_sockets_mutex};
        cv.wait(lk);
      }
    }
    if (!alive)
      break;

    client_sockets_mutex.lock();
    int current{std::move(client_sockets.front())};
    client_sockets.pop();
    client_sockets_mutex.unlock();

    Request req;
    Response res(current);
    try {

      read(current, &req);
      // std::cout << req.getPath() << std::endl;
      m_pathToHandlerMap.at(req.getPath())(req, res);

    } catch (std::out_of_range &e) {

      std::cout << "404 error" << std::endl;
      write(current, resp404.c_str(), resp404.length());

    } catch (std::exception &e) {
      std::cout << "error reading: " << e.what() << std::endl;
      write(current, "Error", sizeof("Error"));
    } catch (...) {
    }
    // std::cout << "---------------------------------" << std::endl;
    // std::cout << req.getMethod() << std::endl;
    // std::cout << req.getPath() << std::endl;
    // req.printHeaders();

    close(current);
  }
}

void Server::read(int _fd, Request *req) {

  std::string message(15000, 0);

  size_t ind = ::read(_fd, &message[0], 15000);

  message.erase(ind, ind + 15000);

  if (ind == 0) {
    throw nullptr;
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

  req->setBody(
      std::string(message, pos + 1, pos + 1 + req->getContentLength()));
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
  {
    std::lock_guard<std::mutex> guard(this->m_acceptor_mutex);
    alive = false;
  }
  cv.notify_all();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  { std::lock_guard<std::mutex> lk(this->m_responder_mutex); }
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
  res = ::listen(m_socket, 100);
  if (res != 0) {
    throw std::runtime_error("error listening");
  }
  int new_socket;

  std::thread responder([&]() { threadWorker(); });
  responder.detach();

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
      tv.tv_usec = 10000;
      FD_SET(m_socket, &rdfs);
      int r = select(m_socket + 1, &rdfs, nullptr, nullptr, &tv);
      if (r == -1) {
        throw std::string("error in select");
      }
      if (r) {

        new_socket = accept(m_socket, nullptr, nullptr);
        client_sockets_mutex.lock();
        client_sockets.push(new_socket);
        client_sockets_mutex.unlock();
        cv.notify_one();
      }
    }
  });

  acceptor.detach();
}
void Server::Get(std::string path, RequestHandler h) {
  string::trim(&path);
  string::tolower(&path);
  if (m_pathToHandlerMap.contains(path)) {
    throw std::invalid_argument("path already exists");
  }
  m_pathToHandlerMap.emplace(path, h);
}
} // namespace http
