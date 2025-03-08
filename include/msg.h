#pragma once
#include <memory>

#include "prelude.h"

class BaseMsg {
 public:
  virtual ~BaseMsg() = default;

  enum TYPE {
    SERVICE,
  };

  TYPE type_;
  service_id service_id_;
  std::shared_ptr<char> buffer_;
  size_t size_;
};