#pragma once
#include <condition_variable>
#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "prelude.h"
#include "service.h"
#include "socket_worker.h"
#include "worker.h"
#include "connect.h"

class Sunnet {
 public:
  static Sunnet& instance();

  void start();
  void wait();

  service_id newService(std::shared_ptr<std::string> type);
  std::shared_ptr<Service> getService(service_id id);
  void killService(service_id id);

  void addReadyService(std::shared_ptr<Service> service);
  std::shared_ptr<Service> getReadyService();

  void send(service_id target, std::shared_ptr<BaseMsg> msg);

  void addConnect(socket_id fd, service_id id, Connect::TYPE type);
  std::shared_ptr<Connect> getConnect(socket_id id);
  bool removeConnect(socket_id id);

  int listenPort(int port, service_id service_id);
  void closeConnect(socket_id fd);

 private:
  Sunnet() = default;

  void startWorker();
  void startSocketWorker();

  // 线程
  const int WORKER_NUMBER = 3;
  std::vector<Worker*> workers_;
  std::vector<std::thread*> threads_;
  SocketWorker* socket_worker_;
  std::thread* socket_thread_;

  // 服务
  std::map<service_id, std::shared_ptr<Service>> services_;
  std::shared_mutex services_mutex_;
  service_id max_service_id_{0};

  // 待处理服务
  std::queue<std::shared_ptr<Service>> ready_services_;
  std::mutex ready_mutex_;
  std::condition_variable ready_cv_;

  // 网络连接
  std::map<socket_id, std::shared_ptr<Connect>> connects_{};
  std::shared_mutex connects_mutex_;
};