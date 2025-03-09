#pragma once
#include <queue>
#include <string>

#include "service_msg.h"
#include "socket_msg.h"
#include "prelude.h"

class Sunnet;
class Worker;

class Service {
  friend Sunnet;
  friend Worker;

 public:
  void onInit();
  void onMsg(std::shared_ptr<BaseMsg> msg);
  void onExit();

 private:
  void pushMsg(std::shared_ptr<BaseMsg> msg);
  std::shared_ptr<BaseMsg> popMsg();
  void processMsg(int max);
  bool processMsg();

  void onServiceMsg(std::shared_ptr<ServiceMsg> msg);
  void onAcceptMsg(std::shared_ptr<SocketAcceptMsg> msg);
  void onRWMsg(std::shared_ptr<SocketRWMsg> msg);
  void onSocketData(socket_id fd, const char* buffer, int len);
  void onSocketWritable(socket_id fd);
  void onSocketClose(socket_id fd);

  service_id id_;
  std::shared_ptr<std::string> type_;

  std::atomic<bool> is_existing_{false};

  bool has_ready_{false};
  std::mutex ready_mutex_;

  std::queue<std::shared_ptr<BaseMsg>> msg_queue_;
  std::mutex queue_mutex_;
};