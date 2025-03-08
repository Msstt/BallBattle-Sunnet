#pragma once
#include <memory>

class Sunnet;
class Service;

class Worker {
 public:
  void operator()();

  int id_;
  int each_num_;

 private:
  void checkService(std::shared_ptr<Service> service);
};