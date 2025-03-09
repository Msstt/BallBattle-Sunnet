#pragma once
#include <memory>

#include "prelude.h"

class BaseMsg {
 public:
  virtual ~BaseMsg() = default;

  enum TYPE { SERVICE, SOCKET_ACCEPT, SOCKET_RW };

  TYPE type_;
  service_id service_id_;
};