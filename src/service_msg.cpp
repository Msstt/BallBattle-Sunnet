#include "service_msg.h"

std::shared_ptr<BaseMsg> ServiceMsg::makeMsg(service_id target, char* buffer,
                                             int len) {
  auto msg = std::make_shared<ServiceMsg>();
  msg->type_ = BaseMsg::TYPE::SERVICE;
  msg->service_id_ = target;
  msg->buffer_ = std::shared_ptr<char>(buffer, std::default_delete<char[]>());
  msg->size_ = len;
  return msg;
}