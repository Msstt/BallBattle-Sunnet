#include "sunnet.h"

Sunnet& Sunnet::instance() {
  static Sunnet instance_;
  return instance_;
}

void Sunnet::startWorker() {
  for (int i = 0; i < this->WORKER_NUMBER; i++) {
    Worker* worker = new Worker();
    worker->id_ = i;
    worker->each_num_ = 1 << i;
    std::thread* thread = new std::thread(*worker);
    this->workers_.push_back(worker);
    this->threads_.push_back(thread);
  }
}

void Sunnet::start() { this->startWorker(); }

void Sunnet::wait() {
  for (auto& thread : this->threads_) {
    thread->join();
  }
}

service_id Sunnet::newService(std::shared_ptr<std::string> type) {
  auto service = std::make_shared<Service>();
  service->id_ = this->max_service_id_++;
  service->type_ = type;
  std::unique_lock lock(this->services_mutex_);
  this->services_[service->id_] = service;
  lock.unlock();
  service->onInit();
  return service->id_;
}

std::shared_ptr<Service> Sunnet::getService(service_id id) {
  std::shared_lock lock(this->services_mutex_);
  if (this->services_.find(id) != this->services_.end()) {
    return this->services_[id];
  }
  return nullptr;
}

void Sunnet::killService(service_id id) {
  auto service = this->getService(id);
  if (service == nullptr) {
    return;
  }
  service->onExit();
  service->is_existing_ = true;
  std::unique_lock lock(this->services_mutex_);
  this->services_.erase(id);
}

void Sunnet::addReadyService(std::shared_ptr<Service> service) {
  std::unique_lock lock(service->ready_mutex_);
  if (service->has_ready_) {
    return;
  }
  std::unique_lock lock2(this->ready_mutex_);
  this->ready_services_.push(service);
  service->has_ready_ = true;
  this->ready_cv_.notify_one();
}

std::shared_ptr<Service> Sunnet::getReadyService() {
  std::unique_lock lock(this->ready_mutex_);
  this->ready_cv_.wait(lock,
                       [this]() { return !this->ready_services_.empty(); });
  auto service = this->ready_services_.front();
  this->ready_services_.pop();
  std::unique_lock lock2(service->ready_mutex_);
  service->has_ready_ = false;
  return service;
}

void Sunnet::Send(service_id target, std::shared_ptr<BaseMsg> msg) {
  auto service = this->getService(target);
  if (service == nullptr) {
    return;
  }
  service->pushMsg(msg);
  this->addReadyService(service);
}