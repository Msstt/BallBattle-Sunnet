#pragma once
#include "msg.h"

class SocketAcceptMsg : public BaseMsg {
 public:
  int listen_fd_;
  int client_fd_;
};

class SocketRWMsg : public BaseMsg {
 public:
  int fd_;
  bool can_read_{false};
  bool can_write_{false};
};