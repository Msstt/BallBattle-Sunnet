#include <iostream>

#include "worker.h"
#include "sunnet.h"

void Worker::operator()() {
  while (true) {
    auto service = Sunnet::instance().getReadyService();
    if (service == nullptr) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    } else {
      service->processMsg(this->each_num_);
      this->checkService(service);
    }
  }
}

void Worker::checkService(std::shared_ptr<Service> service) {
  if (service->is_existing_) {
    return;
  }
  std::unique_lock lock(service->queue_mutex_);
  if (!service->msg_queue_.empty()) {
    Sunnet::instance().addReadyService(service);
  }
}