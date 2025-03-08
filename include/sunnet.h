#pragma once
#include <map>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <condition_variable>

#include "prelude.h"
#include "service.h"
#include "worker.h"

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

  void Send(service_id target, std::shared_ptr<BaseMsg> msg);

 private:
  Sunnet() = default;

  void startWorker();

  const int WORKER_NUMBER = 3;
  std::vector<Worker*> workers_;
  std::vector<std::thread*> threads_;

  std::map<service_id, std::shared_ptr<Service>> services_;
  std::shared_mutex services_mutex_;
  service_id max_service_id_{0};

  std::queue<std::shared_ptr<Service>> ready_services_;
  std::mutex ready_mutex_;
  std::condition_variable ready_cv_;
};