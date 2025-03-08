#pragma once
#include "msg.h"

class ServiceMsg : public BaseMsg {
 public:
  static std::shared_ptr<BaseMsg> makeMsg(service_id target, char* buffer,
                                          int len);
};