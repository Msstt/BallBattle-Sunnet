#include <iostream>

#include "sunnet.h"

void pingTest() {
  auto type = std::make_shared<std::string>("ping");
  auto ping1 = Sunnet::instance().newService(type);
  auto ping2 = Sunnet::instance().newService(type);
  auto pong = Sunnet::instance().newService(type);

  Sunnet::instance().send(
      pong, ServiceMsg::makeMsg(ping1, new char[3]{'h', 'i', '\0'}, 3));
  Sunnet::instance().send(
      pong, ServiceMsg::makeMsg(ping2,
                                new char[6]{'h', 'e', 'l', 'l', 'o', '\0'}, 6));
}

void listenTest() {
  int fd = Sunnet::instance().listenPort(8001, 1);
  std::this_thread::sleep_for(std::chrono::seconds(15));
  Sunnet::instance().closeConnect(fd);
}

void echoTest() {
  Sunnet::instance().newService(std::make_shared<std::string>("gateway"));
}

int main() {
  Sunnet::instance().start();
  echoTest();
  Sunnet::instance().wait();
  return 0;
}