#include "socket_worker.h"

#include <sys/socket.h>

#include <iostream>

#include "socket_msg.h"
#include "sunnet.h"

void SocketWorker::init() {
  this->epoll_fd_ = epoll_create(1024);
  if (this->epoll_fd_ <= 0) {
    std::cout << "create epoll_fd failed" << std::endl;
  }
}

void SocketWorker::operator()() {
  const int EVENT_SIZE = 64;
  while (true) {
    epoll_event events[EVENT_SIZE];
    int count = epoll_wait(this->epoll_fd_, events, EVENT_SIZE, -1);
    for (int i = 0; i < count; i++) {
      onEvent(events[i]);
    }
  }
}

void SocketWorker::addEvent(socket_id fd) {
  epoll_event event;
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = fd;
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1) {
    std::cout << "add epoll event failed" << std::endl;
  }
}

void SocketWorker::removeEvent(socket_id fd) {
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_DEL, fd, NULL) == -1) {
    std::cout << "remove epoll event failed " << errno << std::endl;
  }
}

void SocketWorker::modifyEvent(socket_id fd, bool need_write) {
  epoll_event event;
  event.events = need_write ? EPOLLIN | EPOLLOUT | EPOLLET : EPOLLIN | EPOLLET;
  event.data.fd = fd;
  if (epoll_ctl(this->epoll_fd_, EPOLL_CTL_ADD, fd, &event) == -1) {
    std::cout << "modify epoll event failed" << std::endl;
  }
}

void SocketWorker::onEvent(epoll_event event) {
  auto connect = Sunnet::instance().getConnect(event.data.fd);
  if (connect == nullptr) {
    return;
  }
  bool can_read = event.events & EPOLLIN;
  bool can_write = event.events & EPOLLOUT;
  bool error = event.events & EPOLLERR;
  if (connect->type_ == Connect::TYPE::LISTEN) {
    this->onAccept(connect);
  } else {
    if (can_read || can_write) {
      this->OnRW(connect, can_read, can_write);
    }
  }
}

void SocketWorker::onAccept(std::shared_ptr<Connect> connect) {
  socket_id client = accept(connect->fd_, nullptr, nullptr);
  if (client < 0) {
    return;
  }
  fcntl(client, F_SETFL, O_NONBLOCK);
  unsigned long buffer_size = 2 * 1024 * 1024;
  if (setsockopt(client, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size)) < 0) {
    std::cout << "setsockopt failed: " << errno << std::endl;
  }
  Sunnet::instance().addConnect(client, connect->service_id_,
                                Connect::TYPE::CLIENT);
  this->addEvent(client);

  auto msg = std::make_shared<SocketAcceptMsg>();
  msg->type_ = BaseMsg::TYPE::SOCKET_ACCEPT;
  msg->listen_fd_ = connect->fd_;
  msg->client_fd_ = client;
  Sunnet::instance().send(connect->service_id_, msg);
}

void SocketWorker::OnRW(std::shared_ptr<Connect> connect, bool can_read,
                        bool can_write) {
  auto msg = std::make_shared<SocketRWMsg>();
  msg->type_ = BaseMsg::TYPE::SOCKET_RW;
  msg->fd_ = connect->fd_;
  msg->can_read_ = can_read;
  msg->can_write_ = can_write;
  Sunnet::instance().send(connect->service_id_, msg);
}