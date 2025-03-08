#include "service.h"

#include "sunnet.h"

void Service::onInit() { std::cout << "onInit" << std::endl; }

void Service::onExit() { std::cout << "onExit" << std::endl; }

void Service::onMsg(std::shared_ptr<BaseMsg> msgBase) {
  if (msgBase->type_ == BaseMsg::TYPE::SERVICE) {
    auto msg = std::dynamic_pointer_cast<ServiceMsg>(msgBase);
    std::cout << "[" << this->id_ << "] " << msg->buffer_ << std::endl;
    Sunnet::instance().Send(
        msg->service_id_,
        ServiceMsg::makeMsg(
            this->id_, new char[999999]{'p', 'i', 'n', 'g', '\0'}, 999999));
  } else {
    std::cout << "onMsg" << std::endl;
  }
}

void Service::pushMsg(std::shared_ptr<BaseMsg> msg) {
  std::unique_lock lock(this->queue_mutex_);
  this->msg_queue_.push(msg);
}

std::shared_ptr<BaseMsg> Service::popMsg() {
  std::shared_ptr<BaseMsg> msg = nullptr;
  std::unique_lock lock(this->queue_mutex_);
  if (!this->msg_queue_.empty()) {
    msg = this->msg_queue_.front();
    this->msg_queue_.pop();
  }
  return msg;
}

bool Service::processMsg() {
  auto msg = this->popMsg();
  if (msg) {
    onMsg(msg);
    return true;
  }
  return false;
}

void Service::processMsg(int max) {
  for (int i = 0; i < max; i++) {
    if (!processMsg()) {
      break;
    }
  }
}