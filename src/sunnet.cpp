#include "sunnet.h"

#include <netinet/in.h>
#include <sys/socket.h>

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

void Sunnet::start() {
  signal(SIGPIPE, SIG_IGN);
  this->startWorker();
  this->startSocketWorker();
}

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

void Sunnet::send(service_id target, std::shared_ptr<BaseMsg> msg) {
  auto service = this->getService(target);
  if (service == nullptr) {
    return;
  }
  service->pushMsg(msg);
  this->addReadyService(service);
}

void Sunnet::startSocketWorker() {
  this->socket_worker_ = new SocketWorker();
  this->socket_worker_->init();
  this->socket_thread_ = new std::thread(*this->socket_worker_);
}

void Sunnet::addConnect(socket_id fd, service_id id, Connect::TYPE type) {
  auto connect = std::make_shared<Connect>();
  connect->fd_ = fd;
  connect->service_id_ = id;
  connect->type_ = type;
  std::unique_lock lock(this->connects_mutex_);
  this->connects_[fd] = connect;
}

std::shared_ptr<Connect> Sunnet::getConnect(socket_id id) {
  std::shared_lock lock(this->connects_mutex_);
  if (this->connects_.find(id) != this->connects_.end()) {
    return this->connects_[id];
  }
  return nullptr;
}

bool Sunnet::removeConnect(socket_id id) {
  std::unique_lock lock(this->connects_mutex_);
  return this->connects_.erase(id);
}

int Sunnet::listenPort(int port, service_id service_id) {
  socket_id fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd <= 0) {
    std::cout << "create listen socket failed" << std::endl;
    return -1;
  }
  fcntl(fd, F_SETFL, O_NONBLOCK);
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(fd, (sockaddr*)&address, sizeof(address)) == -1) {
    std::cout << "bind listen socket failed" << std::endl;
  }
  if (listen(fd, 64) < 0) {
    std::cout << "listen " << port << " failed" << std::endl;
  }
  this->addConnect(fd, service_id, Connect::TYPE::LISTEN);
  socket_worker_->addEvent(fd);
  return fd;
}

void Sunnet::closeConnect(socket_id fd) {
  if (this->removeConnect(fd)) {
    socket_worker_->removeEvent(fd);
  }
  close(fd);
}