#pragma once
#include "prelude.h"

class Connect {
 public:
  enum TYPE { LISTEN, CLIENT };

  TYPE type_;
  socket_id fd_;
  service_id service_id_;
};