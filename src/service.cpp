#include "service.h"

#include "sunnet.h"

void Service::onInit() { Sunnet::instance().listenPort(8002, this->id_); }

void Service::onExit() { std::cout << "onExit" << std::endl; }

void Service::onMsg(std::shared_ptr<BaseMsg> msgBase) {
  if (msgBase->type_ == BaseMsg::TYPE::SERVICE) {
    this->onServiceMsg(std::dynamic_pointer_cast<ServiceMsg>(msgBase));
  }
  if (msgBase->type_ == BaseMsg::TYPE::SOCKET_ACCEPT) {
    this->onAcceptMsg(std::dynamic_pointer_cast<SocketAcceptMsg>(msgBase));
  }
  if (msgBase->type_ == BaseMsg::TYPE::SOCKET_RW) {
    this->onRWMsg(std::dynamic_pointer_cast<SocketRWMsg>(msgBase));
  }
}

void Service::onServiceMsg(std::shared_ptr<ServiceMsg> msg) {}

void Service::onAcceptMsg(std::shared_ptr<SocketAcceptMsg> msg) {}

void Service::onRWMsg(std::shared_ptr<SocketRWMsg> msg) {
  if (msg->can_read_) {
    const int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    int len = 0;
    do {
      len = read(msg->fd_, buffer, BUFFER_SIZE);
      if (len > 0) {
        this->onSocketData(msg->fd_, buffer, len);
      }
    } while (len == BUFFER_SIZE);
    if (len <= 0 && errno != EAGAIN) {
      if (Sunnet::instance().getConnect(msg->fd_)) {
        this->onSocketClose(msg->fd_);
        Sunnet::instance().closeConnect(msg->fd_);
      }
    }
  }
  if (msg->can_write_) {
    if (Sunnet::instance().getConnect(msg->fd_)) {
      this->onSocketWritable(msg->fd_);
    }
  }
}

void Service::onSocketData(socket_id fd, const char* buffer, int len) {
  std::cout << buffer << std::endl;
  char write_buffer[3] = {'O', 'K', '\n'};
  write(fd, write_buffer, 3);
}

void Service::onSocketWritable(socket_id fd) {}

void Service::onSocketClose(socket_id fd) {
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