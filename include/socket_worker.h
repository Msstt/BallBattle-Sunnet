#pragma once
#include <libepoll-shim/sys/epoll.h>

#include "prelude.h"
#include "connect.h"

class Sunnet;
class Service;

class SocketWorker {
 public:
  void init();
  void operator()();

  void addEvent(socket_id fd);
  void removeEvent(socket_id fd);
  void modifyEvent(socket_id fd, bool need_write);

 private:
  void onEvent(epoll_event event);
  void onAccept(std::shared_ptr<Connect> connect);
  void OnRW(std::shared_ptr<Connect> connect, bool can_read, bool can_write);

  socket_id epoll_fd_;
};